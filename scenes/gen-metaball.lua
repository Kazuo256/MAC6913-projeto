
local size, step, n, r, b = ...

assert(size and n and r and b)

local bumps = {}

math.randomseed(os.time())
math.random()
math.random()

local function rand()
  local r = 0
  for i=1,5 do
    r = r + math.random()
  end
  return r/5
end

for i=1,n do
  table.insert(bumps, {
    (size-2*r)*(rand() - 0.5),
    (size-2*r)*(rand() - 0.5),
    (size-2*r)*(rand() - 0.5)
  })
end

local output = [[
Shape "metaballsurface"
  "point P" [%s]
  "float R" [%s]
  "float B" [%s]
  "float step" [%f]
  "vector space" [%f %f %f]
]]

local Ps = ""
local Rs = ""
local Bs = ""

for i,bump in ipairs(bumps) do
  Ps = Ps..string.format("%f %f %f  ", unpack(bump))
  Rs = Rs..r.." "
  Bs = Bs..b.." "
end

print(output:format(Ps, Rs, Bs, step, size, size, size))

