# Program that calculates first and follow sets of a given grammar
'''
grammar = [ ['S', 'A', 'C', 'B'],
			['S', 'C', 'b', 'B'],
			['S', 'B', 'a'],
			['A', 'd', 'a'],
			['A', 'B', 'C'],
			['B', 'g'],
			['B', 'epsilon'],
			['C', 'h'],
			['C', 'epsilon'] ]

'''
grammar = [ ['S', 'A', 'B', 'C', 'D', 'E'],
			['A', 'a'],
			['A', 'epsilon'],
			['B', 'b'],
			# ['B', 'epsilon'],
			['C', 'c'],
			['D', 'd'],
			['D', 'epsilon'],
			['E', 'e'],
			['E', 'epsilon'] ]

num_terminals = 0
num_nonterminals = 0
terminals = set()
nonterminals = set()

# Rule 1:
for rule in grammar:
	lhs, rhs = rule[0], rule[1:]
	nonterminals.add(lhs)
	for symbol in rhs:
		if(symbol.isupper()):
			nonterminals.add(symbol)
		elif(symbol.islower()):
			terminals.add(symbol)


print('Terminals:')
print(terminals)
print('Nonterminals:')
print(nonterminals)


firstSets = {}

# Rule 2:

for terminal in terminals:
	fs = set()
	fs.add(terminal)
	firstSets[terminal] = fs

for nonterminal in nonterminals:
	fs = set()
	firstSets[nonterminal] = fs

for rule in grammar:
	if(len(rule) == 2):
		if(rule[1] == 'epsilon'):
			firstSets[rule[0]].add('epsilon')

nullSet = set()
nullSet.add('epsilon')

# Rules 3 and 4

changed = True

while changed:
	firstSetsCopy = firstSets.copy()
	for rule in grammar:
		lhs, rhs = rule[0], rule[1:]
		nullable = True
		for symbol in rhs:
			firstSets[lhs] = firstSets[lhs].union(firstSets[symbol].difference(nullSet))
			if('epsilon' not in firstSets[symbol]):
				nullable = False
				break
			else:
				continue
		if(nullable):
			firstSets[lhs] = firstSets[lhs].union(nullSet)
	if(firstSetsCopy == firstSets):
		changed = False

print(firstSets)


# Calculating follow sets

followSets = {nonterminal : set() for nonterminal in nonterminals}
followSets['S'].add('$')

changed = True

while changed:
	followSetsCopy = followSets.copy()
	for rule in grammar:
		lhs, rhs = rule[0], rule[1:]
		nullable = False
		if(rhs[-1].isupper()):
			followSets[rhs[-1]] = followSets[rhs[-1]].union(followSets[lhs])
			if 'epsilon' in firstSets[rhs[-1]]:
				nullable = True
			tempFirstSet = firstSets[rhs[-1]].difference(nullSet)
			n_nt = len(rhs)
			i = n_nt - 2
			while i >= 0:
				if(rhs[i].isupper()):
					followSets[rhs[i]] = followSets[rhs[i]].union(tempFirstSet)
					if(nullable):
						followSets[rhs[i]] = followSets[rhs[i]].union(followSets[lhs])
					if 'epsilon' in firstSets[rhs[i]] :
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