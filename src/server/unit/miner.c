#include "../../config.h"
#include "../server.h"
#include "../unit.h"
#include "miner.h"

int unit_miner_special_function(UnitEntry *unit) {
	/* Add money for miners */
	if(unit->powered) {
		if(ss->team[unit->team].miner_income_counter++ >= INCOME_PER_MONEY) {
			ss->team[unit->team].money++;
			ss->team[unit->team].miner_income_counter = 0;
		}
	}
	
	return 0;
}
