/*
1€ Arduino implementation: https://gery.casiez.net/1euro/

It uses a first order low-pass filter with an adaptive cutoff frequency: at low speeds,
a low cutoff stabilizes the signal by reducing jitter, but as speed increases, the cutoff is increased to reduce lag.

2 tunable parameters. beta and fcmin.
beta is 0 at first and fcmin(min cut off) is 1 Hz. 

Note that parameters fcmin and beta have clear conceptual relationships: if high speed lag is a problem, increase beta; if slow speed jitter is a problem, decrease fcmin.

#################################

Gyroskop ist für Start der Bewegungswahrnehmung eingentlich egal. Grundsätzlich ist mehr oder weniger nur die Geschwindigkeit wichtig.

*/

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
