/*
ZXTune foobar2000 decoder component by djdron (C) 2013 - 2026

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#import <Cocoa/Cocoa.h>

#include <SDK/foobar2000-all.h>

#include <core/plugins/player_plugin.h>

namespace ZXTune
{
const std::vector<PlayerPlugin::Ptr>& PlayerPlugins();
bool PlayerPluginEnabled(const PlayerPlugin::Ptr& pp);
void SetPlayerPluginsDisabled(const std::vector<PlayerPlugin::Ptr>& players_disabled);
}
//namespace ZXTune

@interface CheckboxTableCellView : NSTableCellView
@property (nonatomic, strong) IBOutlet NSButton* check;
@end

@implementation CheckboxTableCellView
@end

@interface MyTableView : NSTableView
@end

@implementation MyTableView
- (void)keyDown:(NSEvent *)event {
	NSString *characters = [event charactersIgnoringModifiers];
	if ([characters isEqualToString:@" "]) {
		if(self.numberOfSelectedRows && self.doubleAction) {
			[self sendAction:self.doubleAction to:self.target];
			return;
		}
	}
	[super keyDown:event];
}
@end

@interface foo_preferences_mac : NSViewController
@property (nonatomic, strong) IBOutlet NSTableView* table;
@end

@implementation foo_preferences_mac

std::set<int> players_disabled;

- (void)viewDidLoad {
    [super viewDidLoad];
}

- (instancetype)init {
    self = [self initWithNibName: @"foo_preferences_mac" bundle:[NSBundle bundleForClass: [self class]]];
    [self loadSettings];
    return self;
}

- (void) loadSettings {
	ZXTune::PlayerPlugin::Enumerate(); // gather all plugins if not gathered yet
	for(auto p = ZXTune::PlayerPlugins().begin(); p != ZXTune::PlayerPlugins().end(); ++p)
	{
		if(!ZXTune::PlayerPluginEnabled(*p))
			players_disabled.insert(int(p - ZXTune::PlayerPlugins().begin()));
	}
}

- (void) storeSettings {
	std::vector<ZXTune::PlayerPlugin::Ptr> pps;
	for(auto& i : players_disabled)
	{
		assert(i >= 0 && i < (int)ZXTune::PlayerPlugins().size());
		pps.push_back(ZXTune::PlayerPlugins()[i]);
	}
	ZXTune::SetPlayerPluginsDisabled(pps);
}

- (IBAction)onCheckState:(id)sender {
	if([sender state])
		players_disabled.erase(int([sender tag]));
	else
		players_disabled.insert(int([sender tag]));
	[self storeSettings];
}

-(IBAction)onDoubleClick:(id)sender
{
	if(!self.table.numberOfSelectedRows)
		return;
	NSIndexSet *selectedIndexes = [self.table selectedRowIndexes];
	__block bool on = false;
	[selectedIndexes enumerateIndexesUsingBlock:^(NSUInteger idx, BOOL *stop) {
		if(players_disabled.find(int(idx)) != players_disabled.end()) {
			on = true;
			*stop = YES;
		}
	}];
	[selectedIndexes enumerateIndexesUsingBlock:^(NSUInteger idx, BOOL *stop) {
		if(on)
			players_disabled.erase(int(idx));
		else
			players_disabled.insert(int(idx));
		CheckboxTableCellView *cell = [self.table viewAtColumn:0 row:idx makeIfNecessary:NO];
		cell.check.state = on;
	}];
	[self storeSettings];
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView {
	return ZXTune::PlayerPlugins().size();
}

-(NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row {
	
	auto pp = ZXTune::PlayerPlugins()[row];

	NSString *identifier = tableColumn.identifier;
	NSTableCellView *cell = [tableView makeViewWithIdentifier:identifier owner:self];
	if ([identifier isEqualToString:@"onColumn"]) {
		if([cell isKindOfClass:[CheckboxTableCellView class]])
		{
			CheckboxTableCellView* cbcell = (CheckboxTableCellView*)cell;
			[cbcell.check setTag:row];
			cbcell.check.state = players_disabled.find(int(row)) == players_disabled.end();
		}
	} else if ([identifier isEqualToString:@"idColumn"]) {
		cell.textField.stringValue = [[NSString alloc] initWithBytes:pp->Id().data() length:pp->Id().length() encoding:NSUTF8StringEncoding];
	} else if ([identifier isEqualToString:@"descColumn"]) {
		cell.textField.stringValue = [[NSString alloc] initWithBytes:pp->Description().data() length:pp->Description().length() encoding:NSUTF8StringEncoding];
	}
	return cell;
}

@end


class preferences_page_myimpl : public preferences_page
{
public:
	service_ptr instantiate() override {
		return fb2k::wrapNSObject( [ foo_preferences_mac new ] );
	}
	virtual const char * get_name() override { return "ZXTune"; }
	virtual GUID get_guid() override
	{
		static const GUID guid = { 0xce451919, 0xd1db, 0x44ed, { 0x8e, 0x6a, 0x38, 0x87, 0xa7, 0x56, 0xfb, 0x81 } };
		return guid;
	}
	virtual GUID get_parent_guid() override { return guid_input; }
};
    
FB2K_SERVICE_FACTORY(preferences_page_myimpl);
