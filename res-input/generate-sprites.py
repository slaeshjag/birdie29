#!/usr/bin/env python

for team in range(0, 4):
	for var in range(0, 3):
		f = open("../res/team{}_var{}.spr".format(team, var), "w")
		f.write("res/player_{}.png 48 48\n".format(var));
		for i in range(0, 4):
			f.write("D\n");
			f.write("T 200 {}\n".format(i*4 + team));
			f.write("E\n");
		f.close()

