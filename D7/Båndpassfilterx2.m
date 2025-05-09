N=512; % Oppløysing numerisk frekvensakse
b=[1];
a=[1 2/5*(1-sqrt(5)) -(0.8^2)]; % Filterkoeff. IIR
b2 = [1];
a2 = [1 2/5*(sqrt(5)-1) -(0.8^2)];
[H,omg]=freqz(b,a,N);
[H2, omg2] = freqz(b2,a2,N);
figure(1)
subplot(3,1,1)
hold on
plot(omg,abs(H)) % Magnituderespons (lineær)
plot(omg2, abs(H2))
hold off
grid on
ylabel('|H(e^{jω})|')
subplot(3,1,2);
plot(omg,abs(H).^2) % Magnituderespons (kvadratisk)
grid on
ylabel('|H(e^{jω})| [dB]')
subplot(3,1,3);
plot(omg,20*log10(abs(H))) % Faserespons (fase)
grid on
ylabel('∠(H(e^{jω})) [°]')
%figure(2)
%zplane(b,a) % z-diagram (pol-nullpunkt)