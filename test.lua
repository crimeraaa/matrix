matrix = require "matrix"
a = matrix.new(3, 2)
b = matrix.new{1,2,3,4}
c = matrix.new {{1,2,3}, {4,5,6}}

local acc = 1
for i = 1, a.rows do
    for j = 1, a.cols do
        a:set(i, j, acc)
        acc = acc + 1
    end
end

print(a)
print(b)
print(c)

local function bad()
    local d = matrix.new {{1,2,3}, {4,5,6,7}}
    print(d)
end

print(pcall(bad))
-- a:set(1, 1, 4)
-- print(a, a:rows(), a:cols())
