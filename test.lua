matrix = require "matrix"
a = matrix.new(3, 2)

local acc = 1
for i = 1, a:rows() do
    for j = 1, a:cols() do
        a:set(i, j, acc)
        acc = acc + 1
    end
end
-- a:set(1, 1, 4)
-- print(a, a:rows(), a:cols())
