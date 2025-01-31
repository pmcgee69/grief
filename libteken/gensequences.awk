#!/usr/bin/awk -f

#-
# Copyright (c) 2008-2009 Ed Schouten <ed@FreeBSD.org>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
# $FreeBSD: head/sys/teken/gensequences 333925 2018-05-20 14:21:20Z dumbbell $

function die(msg) {
	print msg;
	exit 1;
}

function cchar(str) {
	if (str == "^[")
		return "\\x1B";
	if (str == "SP")
		return " ";

	return str;
}

function csequence(str) {
	if (str == "SP")
		return " ";

	return str;
}

BEGIN {
FS = "\t+"

while (getline > 0) {
	if (NF == 0 || $1 ~ /^#/)
		continue;

	if (NF != 3 && NF != 4)
		die("Invalid line layout: " NF " columns");

	split($3, sequence, " +");
	nsequences = 0;
	for (s in sequence)
		nsequences++;

	prefix = "";
	l_prefix_name[""] = "teken_state_init";
	for (i = 1; i < nsequences; i++) {
		n = prefix csequence(sequence[i]);
		l_prefix_parent[n] = prefix;
		l_prefix_suffix[n] = sequence[i];
		if (!l_prefix_name[n])
			l_prefix_name[n] = "teken_state_" ++npr;
		prefix = n;
	}

	suffix = sequence[nsequences];
	cmd = prefix suffix;

	# Fill lists
	if (l_cmd_name[cmd] != "")
		die(cmd " already exists");
	l_cmd_prefix[cmd] = prefix;
	l_cmd_suffix[cmd] = suffix;
	l_cmd_args[cmd] = $4;
	l_cmd_abbr[cmd] = $1;
	l_cmd_name[cmd] = $2;
	l_cmd_c_name[cmd] = "teken_subr_" tolower($2);
	gsub(" ", "_", l_cmd_c_name[cmd]);

	if ($4 != "")
		l_prefix_numbercmds[prefix]++;
}

print "/* Generated file. Do not edit. */";
print "";

for (p in l_prefix_name) {
	if (l_prefix_name[p] != "teken_state_init")
		print "static teken_state_t	" l_prefix_name[p] ";";
}

for (p in l_prefix_name) {
	print "";
	print "/* '" p "' */";
	print "static void";
	print l_prefix_name[p] "(teken_t *t, teken_char_t c)";
	print "{";

	if (l_prefix_numbercmds[p] > 0) {
		print "";
		print "\tif (teken_state_numbers(t, c))";
		print "\t\treturn;";
	}

	print "";
	print "\tswitch (c) {";
	for (c in l_cmd_prefix) {
		if (l_cmd_prefix[c] != p)
			continue;

		print "\tcase '" cchar(l_cmd_suffix[c]) "': /* " l_cmd_abbr[c] ": " l_cmd_name[c] " */";

		if (l_cmd_args[c] == "v") {
			print "\t\t" l_cmd_c_name[c] "(t, t->t_curnum, t->t_nums);";
		} else {
			printf "\t\t%s(t", l_cmd_c_name[c];
			split(l_cmd_args[c], args, " ");
			for (a = 1; args[a] != ""; a++) {
				if (args[a] == "n")
					printf ", (t->t_curnum < %d || t->t_nums[%d] == 0) ? 1 : t->t_nums[%d]", a, (a - 1), (a - 1);
				else if (args[a] == "r")
					printf ", t->t_curnum < %d ? 0 : t->t_nums[%d]", a, (a - 1);
				else
					die("Invalid argument type: " args[a]);
			}
			print ");";
		}
		print "\t\tbreak;";
	}
	for (pc in l_prefix_parent) {
		if (l_prefix_parent[pc] != p)
			continue;
		print "\tcase '" cchar(l_prefix_suffix[pc]) "':";
		print "\t\tteken_state_switch(t, " l_prefix_name[pc] ");";
		print "\t\treturn;";
	}

	print "\tdefault:";
	if (l_prefix_name[p] == "teken_state_init") {
		print "\t\tteken_subr_regular_character(t, c);";
	} else {
		print "\t\tteken_printf(\"Unsupported sequence in " l_prefix_name[p] ": %u\\n\", (unsigned int)c);";
	}
	print "\t\tbreak;";

	print "\t}";

	if (l_prefix_name[p] != "teken_state_init") {
		print "";
		print "\tteken_state_switch(t, teken_state_init);";
	}
	print "}";
}

}
