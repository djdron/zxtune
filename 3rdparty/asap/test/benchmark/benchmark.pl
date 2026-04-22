use Time::HiRes qw(time);
use strict;

print q{File                          ,gcc64,gcc32,vc64 ,vc32 ,D    ,C#   ,Java ,Swift,Node ,Pytho,GME   ,SAP
};
my @progs = (
	'win32/x64/asapconv.exe -o .wav',
	'win32/asapconv.exe -o .wav',
	'win32/msvc/x64/asapconv.exe -o .wav',
	'win32/msvc/asapconv.exe -o .wav',
	'd/asap2wav.exe',
	'csharp/asap2wav.exe',
	'java -jar java/asap2wav.jar',
	'swift/asap2wav.exe',
	'node javascript/asap2wav.js',
	'python python/asap2wav.py',
	'test/benchmark/gme_benchmark.exe',
	'test/benchmark/sap_benchmark.exe'
);
for my $file (glob 'test/benchmark/*.sap') {
	printf '%30s', $file =~ m{([^/]+)$};
	prog: for my $prog (@progs) {
		my @cmd = (split(/ /, $prog), $file);
		my $time = time;
		my $COUNT = 3;
		print STDERR "@cmd\n";
		for my $i (1 .. $COUNT) {
			unless (system(@cmd) == 0) {
				print ',ERROR';
				next prog;
			}
		}
		$time = (time() - $time) / $COUNT;
		printf ',%5.2f', $time;
	}
	print "\n";
}
