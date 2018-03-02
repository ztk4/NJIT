function pl = to_planck_len(meters)

if meters < 0
  error('Cannot have a negative length');
end

h = 6.62607004e-34;
G = 6.67408e-11;
c = physconst('LightSpeed');

pl = sqrt(2 * pi * c^3 / (h * G)) * meters;
end