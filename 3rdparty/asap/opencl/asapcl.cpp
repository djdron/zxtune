/*
 * asapcl.cpp - converter of ASAP-supported formats to WAV files
 *
 * Copyright (C) 2021-2026  Piotr Fusik
 *
 * This file is part of ASAP (Another Slight Atari Player),
 * see http://asap.sourceforge.net
 *
 * ASAP is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * ASAP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ASAP; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <vector>

#define CL_TARGET_OPENCL_VERSION 200
#include <CL/cl.h>

#include "asap.h"

void check_error(int err)
{
	if (err != CL_SUCCESS) {
		fprintf(stderr, "OpenCL error %d\n", err);
		exit(1);
	}
}

struct ASAP_Workload
{
	cl_long wav_offset;
	cl_int wav_len;
	cl_int filename_offset;
	cl_int module_offset;
	cl_int module_len;
	cl_int song;
	cl_int duration;
	ASAP_Workload() = default;
	ASAP_Workload(cl_long wav_offset, cl_int wav_len, cl_int filename_offset, cl_int module_offset, cl_int module_len, cl_int song, cl_int duration)
		: wav_offset(wav_offset), wav_len(wav_len), filename_offset(filename_offset), module_offset(module_offset), module_len(module_len), song(song), duration(duration) {
	}
};

int main(int argc, char **argv)
{
	cl_platform_id platform;
	cl_uint num;
	check_error(clGetPlatformIDs(1, &platform, &num));
	if (num == 0) {
		fprintf(stderr, "No OpenCL platforms\n");
		return 1;
	}

	cl_device_id device;
	check_error(clGetDeviceIDs(platform, CL_DEVICE_TYPE_DEFAULT, 1, &device, &num));
	if (num == 0) {
		fprintf(stderr, "No OpenCL device\n");
		return 1;
	}

	size_t size;
	check_error(clGetDeviceInfo(device, CL_DEVICE_NAME, 0, nullptr, &size));
	std::unique_ptr<char[]> name(new char[size]);
	check_error(clGetDeviceInfo(device, CL_DEVICE_NAME, size, name.get(), nullptr));
	fprintf(stderr, "Running on %s\n", name.get());

	const cl_context_properties properties[] = {
		CL_CONTEXT_PLATFORM, (cl_context_properties) platform, 0
	};
	cl_int err;
	cl_context context = clCreateContext(properties, 1, &device, nullptr, nullptr, &err);
	check_error(err);

	const char *source =
#include "asap-cl.h"
		;
	cl_program program = clCreateProgramWithSource(context, 1, &source, nullptr, &err);
	check_error(err);

	err = clBuildProgram(program, 1, &device, "-cl-std=CL2.0", nullptr, nullptr);
	if (err == CL_BUILD_PROGRAM_FAILURE) {
		check_error(clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &size));
		std::unique_ptr<char[]> log(new char[size]);
		check_error(clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, size, log.get(), nullptr));
		fprintf(stderr, "OpenCL build error:\n%s", log.get());
		return 1;
	}
	check_error(err);

	int exit_code = 0;

	fprintf(stderr, "Loading files\n");
	std::vector<char> filenames;
	std::vector<uint8_t> modules;
	std::vector<ASAP_Workload> workloads;
	size_t wavs_len = 0;

	ASAPInfo *info = ASAPInfo_New();
	for (int argi = 1; argi < argc; argi++) {
		const char *input_file = argv[argi];
		FILE *fp = fopen(input_file, "rb");
		if (fp == NULL) {
			fprintf(stderr, "Cannot open %s\n", input_file);
			exit_code = 1;
			continue;
		}
		static uint8_t module[ASAPInfo_MAX_MODULE_LENGTH];
		int module_len = fread(module, 1, sizeof(module), fp);
		fclose(fp);

		if (!ASAPInfo_Load(info, input_file, module, module_len)) {
			fprintf(stderr, "%s: unsupported file\n", input_file);
			exit_code = 1;
			continue;
		}
		int channels = ASAPInfo_GetChannels(info);
		int songs = ASAPInfo_GetSongs(info);

		for (int song = 0; song < songs; song++) {
			int duration = ASAPInfo_GetDuration(info, song);
			if (duration < 0)
				duration = 180 * 1000;
			int wav_len = 44 + duration * (ASAP_SAMPLE_RATE / 100) / 10 * channels * 2;
			workloads.emplace_back(wavs_len, wav_len, filenames.size(), modules.size(), module_len, songs == 1 ? -1 : song, duration);
			wavs_len += wav_len;
		}
		filenames.insert(filenames.end(), input_file, input_file + strlen(input_file) + 1);
		modules.insert(modules.end(), module, module + module_len);
	}
	ASAPInfo_Delete(info);
	fprintf(stderr, "Emulating %zu songs\n", workloads.size());

	cl_kernel kernel = clCreateKernel(program, "asap2wav", &err);
	check_error(err);

	cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, nullptr, &err);
	check_error(err);

	cl_mem filenames_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR | CL_MEM_HOST_NO_ACCESS, filenames.size(), filenames.data(), &err);
	check_error(err);
	check_error(clSetKernelArg(kernel, 0, sizeof(filenames_buffer), &filenames_buffer));

	cl_mem modules_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR | CL_MEM_HOST_NO_ACCESS, modules.size(), modules.data(), &err);
	check_error(err);
	check_error(clSetKernelArg(kernel, 1, sizeof(modules_buffer), &modules_buffer));

	cl_mem wavs_buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_HOST_READ_ONLY, wavs_len, nullptr, &err);
	check_error(err);
	check_error(clSetKernelArg(kernel, 2, sizeof(wavs_buffer), &wavs_buffer));

	cl_mem workloads_buffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR | CL_MEM_HOST_NO_ACCESS, workloads.size() * sizeof(ASAP_Workload), workloads.data(), &err);
	check_error(err);
	check_error(clSetKernelArg(kernel, 3, sizeof(workloads_buffer), &workloads_buffer));

	const size_t dim = workloads.size();
	check_error(clEnqueueNDRangeKernel(queue, kernel, 1, nullptr, &dim, nullptr, 0, nullptr, nullptr));

	std::unique_ptr<uint8_t[]> wavs(new uint8_t[wavs_len]);
	check_error(clEnqueueReadBuffer(queue, wavs_buffer, false, 0, wavs_len, wavs.get(), 0, nullptr, nullptr));

	check_error(clFinish(queue));
	check_error(clReleaseMemObject(workloads_buffer));
	check_error(clReleaseMemObject(wavs_buffer));
	check_error(clReleaseMemObject(modules_buffer));
	check_error(clReleaseMemObject(filenames_buffer));
	check_error(clReleaseCommandQueue(queue));
	check_error(clReleaseKernel(kernel));
	check_error(clReleaseProgram(program));
	check_error(clReleaseContext(context));

	for (const ASAP_Workload &workload : workloads) {
		const char *input_file = filenames.data() + workload.filename_offset;
		const uint8_t *wav = wavs.get() + workload.wav_offset;
		if (wav[0] != 'R') {
			fprintf(stderr, "%s: conversion error\n", input_file);
			exit_code = 1;
			continue;
		}

		char output_file[FILENAME_MAX];
		int input_file_len = strrchr(input_file, '.') - input_file;
		int song = workload.song;
		int output_file_len = song < 0 ? snprintf(output_file, sizeof(output_file), "%.*s.wav", input_file_len, input_file)
			: snprintf(output_file, sizeof(output_file), "%.*s (song %d).wav", input_file_len, input_file, song + 1);
		if (output_file_len >= static_cast<int>(sizeof(output_file))) {
			fprintf(stderr, "%s: filename too long\n", input_file);
			exit_code = 1;
			continue;
		}

		FILE *fp = fopen(output_file, "wb");
		if (fp == NULL) {
			perror(output_file);
			exit_code = 1;
			break;
		}
		fprintf(stderr, "Writing %s\n", output_file);
		size_t written = fwrite(wav, workload.wav_len, 1, fp);
		if (fclose(fp) != 0 || written != 1) {
			perror(output_file);
			exit_code = 1;
		}
	}
	return exit_code;
}
