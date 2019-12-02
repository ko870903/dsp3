import sys

fin = open(sys.argv[1], 'r', encoding = 'BIG5-HKSCS')
table = {}
for line in fin:
	word = line.split(' ')
	BIG5 = word[0]
	ZhuYin = word[1].split('/')
	for zy in ZhuYin:
		if zy[0] not in table:
			table[zy[0]] = []
		table[zy[0]].append(BIG5)
	table[BIG5] = [BIG5]
fin.close()

fout = open(sys.argv[2], 'w', encoding = 'BIG5-HKSCS')
for key in table.keys():
	fout.write(key)
	for value in table[key]:
		fout.write(' ' + value)
	fout.write('\n')
fout.close()