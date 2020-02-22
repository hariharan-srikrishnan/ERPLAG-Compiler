# Program that calculates first and follow sets of a given grammar

grammar = []
content = None
with open('grammar.txt', 'r') as f:
	content = f.read().splitlines()

for line in content:
	grammar.append(line.split())

terminals = set()

nt = ["program", 
"moduleDeclarations",
"moduleDeclaration",
"otherModules",
"driverModule",
"module",
"ret",
"input_plist",
"N1",
"output_plist",
"N2",
"dataType",
"range_arrays",
"type",
"moduleDef",
"statements",
"statement",
"ioStmt",
"boolConstt",
"var_id_num",
"var",
"whichId",
"simpleStmt",
"assignmentStmt",
"whichStmt",
"lvalueIDStmt",
"lvalueARRStmt",
"index",
"moduleReuseStmt",
"optional",
"idList",
"N3",
"expression",
"U",
"new_NT",
"unary_op",
"arithmeticOrBooleanExpr",
"N7",
"anyTerm",
"N8",
"arithmeticExpr",
"N4",
"term",
"N5",
"factor",
"op1",
"op2",
"logicalOp",
"relationalOp",
"declareStmt",
"conditionalStmt",
"caseStmts",
"N9",
"value",
"default",
"iterativeStmt",
"range"]

nonterminals = set(nt)

for rule in grammar:
	lhs, rhs = rule[0], rule[1:]
	for symbol in rhs:
		if(symbol[0].isupper() and symbol not in nonterminals):
			terminals.add(symbol)

print('Terminals:')
print(terminals)
print('Nonterminals:')
print(nonterminals)


firstSets = {}

# Rule 1:
for terminal in terminals:
	fs = set()
	fs.add(terminal)
	firstSets[terminal] = fs

for nonterminal in nonterminals:
	fs = set()
	firstSets[nonterminal] = fs

# Rule 2:
for rule in grammar:
	if(len(rule) == 2):
		if(rule[1] == 'EPSILON'):
			firstSets[rule[0]].add('EPSILON')

nullSet = set()
nullSet.add('EPSILON')

# Rules 3 and 4

changed = True

while changed:
	firstSetsCopy = firstSets.copy()
	for rule in grammar:
		lhs, rhs = rule[0], rule[1:]
		nullable = True
		for symbol in rhs:
			firstSets[lhs] = firstSets[lhs].union(firstSets[symbol].difference(nullSet))
			if('EPSILON' not in firstSets[symbol]):
				nullable = False
				break
			else:
				continue
		if(nullable):
			firstSets[lhs] = firstSets[lhs].union(nullSet)
	if(firstSetsCopy == firstSets):
		changed = False

print('First Sets:')
print(firstSets)

# Calculating follow sets

followSets = {nonterminal : set() for nonterminal in nonterminals}
followSets['program'].add('$')

changed = True

while changed:
	followSetsCopy = followSets.copy()
	for rule in grammar:
		lhs, rhs = rule[0], rule[1:]
		nullable = False
		if(rhs[-1] in nonterminals):
			followSets[rhs[-1]] = followSets[rhs[-1]].union(followSets[lhs])
			if 'EPSILON' in firstSets[rhs[-1]]:
				nullable = True
		tempFirstSet = firstSets[rhs[-1]].difference(nullSet)
		n_nt = len(rhs)
		i = n_nt - 2
		while i >= 0:
			if(rhs[i] in nonterminals):
				followSets[rhs[i]] = followSets[rhs[i]].union(tempFirstSet)
				if(nullable):
					followSets[rhs[i]] = followSets[rhs[i]].union(followSets[lhs])
				if 'EPSILON' in firstSets[rhs[i]] :
					tempFirstSet = tempFirstSet.union(firstSets[rhs[i]].difference(nullSet))
				else:
					tempFirstSet = firstSets[rhs[i]].difference(nullSet)
					nullable = False
			else:
				tempFirstSet = firstSets[rhs[i]].difference(nullSet)
				nullable = False


			i = i - 1
	if(followSetsCopy == followSets):
		changed = False
	
print('Follow Sets:')
print(followSets)
