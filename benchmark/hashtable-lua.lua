function benchmark()
	words = {}

	for line in io.lines("words.txt") do
		for word in string.gmatch(line, "%S+") do
			if words[word] then
				words[word] = words[word] + 1
			else
				words[word] = 1
			end
		end
	end

	total = 0
	sum = 0
	for word, count in pairs(words) do
		total = total + 1
		sum = sum + count
	end

	print(total)
	print(sum)
end

-- Note: os.clock returns elapsed CPU time, not wall-clock time.
start = os.clock()
benchmark()
stop = os.clock()

print (stop - start)
