matrix = require "matrix"
a = matrix.new(3, 2)
b = matrix.new{1,2,3,4}

-- 2x3
c = matrix.new {
    {1,2,3},
    {4,5,6},
}

-- 3x2
d = matrix.new {
    {1,2},
    {3,4},
    {5,6},
}

local acc = 1
for i = 1, a.rows do
    for j = 1, a.cols do
        a:set(i, j, acc)
        acc = acc + 1
    end
end

print('a:', a)
print('b:', b)
print('c:', c)
print('d:', d)

print(pcall(function()
    local d = matrix.new {{1,2,3}, {4,5,6,7}}
    print(d)
end))
-- a:set(1, 1, 4)
-- print(a, a:rows(), a:cols())
