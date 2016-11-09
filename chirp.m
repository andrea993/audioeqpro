SR=44100;
FN=SR/2;
D=5;
L=D*SR;

t=(0:L-1)/SR;
fy=FN/D*t;
y=sin(2*pi*fy/2.*t);
plot(t,y);

audiowrite('chirp.wav',int16([y' y']*6000),SR,'BitsPerSample',16);
