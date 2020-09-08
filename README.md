# GolfGame1989

Demo Videos on YouTube - https://www.youtube.com/playlist?list=PLKMR2Ppf1A4jcWj0OhxXVMuNQZNznrlG9

This is golf game based on the double pendulum model in professor Theodore P. Jorgensen’s “The Physics of Golf” using the DirectX Tool Kit. The games features that have been implemented include character select, environment select, and a full golf bag with 14 clubs to select

<p align="center">
  <img width="600" src="https://github.com/lehrj/GolfGame1989/blob/master/Images/BallFlight.png">
</p>

### Character Select Features: 
Arm length, arm mass, arm balance point, arm mass moment of inertia, and club length modification for each character

<p align="center">
  <img width="600" src="https://github.com/lehrj/GolfGame1989/blob/master/Images/CharacterSelect.png">
</p>

### Environment Select Features:
Air density, gravity, wind values and terrain color. An Alien non-Earth like environment has been added for fun to demonstrate how wildly different environmental variables can affect the ball flight… plus space golf is cool, just ask Alan Shepard

<p align="center">
  <img width="600" src="https://github.com/lehrj/GolfGame1989/blob/master/Images/EnvironmentSelect.png">
</p>

### Club Features:
Angle, balance point, coefficient of restitution, length, mass, mass moment of inertia

<p align="center">
  <img width="600" src="https://github.com/lehrj/GolfGame1989/blob/master/Images/SwingView.png">
</p>

### Gameplay:
Currently the game play includes a 3 click power bar swing. The first click starts the swing, the second click sets the backswing percentage to control power, and the third click sets the impact. The closer the impact is set to the position indicated on the UI the more square the club face will be at impact. Setting the impact point early or late will cause the club face to be more closed or open at impact. This will result in the ball flight to hook or slice in the respected direction

### Future mechanics:
Currently the ball placement angle respective to the player is hard coded, future update will include the ability to position the player forward or back along the swing plane which will result an adjustment in the vertical club face angle at impact.
Future update will allow the adjustment of the direction of travel of the club’s swing plan to be from out to in, in to in, and in to out which will result in adjustments to the ball flight path. This combined with the impact face angle will result in all 9 traditional golf flight paths to be simulated.
Power bar tolerance update will penalize very early or late impacts with “skull” and “duff” ball flight. This will be modeled on the bottom of the club impacting the ball or turf dug up by a divot impacting the ball. 


### Controls:

Spacebar : Triple click gameplay button. Start swing, set power, and set impact

, : Turn shot aim left

. : Turn shot aim right

] : Select next club

[ : Select previous club

1-0 : Select one of the first 10 clubs in the bag as related to key

V : Reset swing at tee position and set camera to aiming postion

B : Reset swing to last ball position and set camera to aiming position

Z : Debug test swing, 100% power & perfect impact

Y : Enter first person camera

W : First person camera move forward

A : First person camera strafe left

S : First person camera move back

D : First person camera strafe right

Q : First person camera turn left

E : First person camera turn right

F : First person camera move up

C : First person camera move down

Mouse Button 1, hold : While in first person camera turn camera




### Works Cited:

Bourg and Baywalec. (2013). Physics for Game Developers. CA USA. O’Reilly

Gardiner, M. P. The Physics of Golf. https://www.golf-simulators.com/physics.htm

Jorgensen, T. P. (1999). The Physics of Golf. NY. Springer Science+Business Media

Palmer, G. (2005). Physics for Game Programmers. NY NY. Springer-Verlag

Penner, A. R. (2003). The Physics of Golf, Reports on Progress in Physics vol 66 p 131 – 171

Penner, A.R. (2002). The Run of a Golf Ball, NRC Research Press Web Site http://cjp.nrc.ca/, p 931 - 940

Roh, W.J., Lee, C.W. (2010). Golf Ball Landing, Bounce and Roll on Turf. 8th Conference of the International Sports Engineering Association. P 3237 - 3242


### Music Credits:

Music01 = Techno DRIVE!!! by Centurion_of_war. https://opengameart.org/content/techno-drive

Music02 = Snabish Tunes 8 Bit Version by Snabisch. https://opengameart.org/content/snabisch-tunes-8-bit-version

Music03 = In Summer, NES Version by Snabisch. https://opengameart.org/content/in-summer-nes-version

Music04 = Retroracing Beach by Bogart VGM. https://opengameart.org/content/retroracing-beach

