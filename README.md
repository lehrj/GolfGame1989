# GolfGame1989
This is golf game based on the double pendulum model in professor Theodore P. Jorgensen’s “The Physics of Golf” using the DirectX Tool Kit.  
The games features that have been implemented include character select, environment select, a full golf bag with 14 clubs to select

<p align="center">
  <img width="600" src="https://github.com/lehrj/GolfGame1989/blob/master/Images/BallFlight.png">
</p>

Character select features: 
Arm length, arm mass, arm balance point, arm mass moment of inertia, and club length modification for each character

<p align="center">
  <img width="600" src="https://github.com/lehrj/GolfGame1989/blob/master/Images/CharacterSelect.png">
</p>

Environment select features:
Air density, gravity, wind values and terrain color

<p align="center">
  <img width="600" src="https://github.com/lehrj/GolfGame1989/blob/master/Images/EnvironmentSelect.png">
</p>

Club features:
Angle, balance point, coefficient of restitution, length, mass, mass moment of inertia

<p align="center">
  <img width="600" src="https://github.com/lehrj/GolfGame1989/blob/master/Images/SwingView.png">
</p>

Gameplay:
Currently the game play includes a 3 click power bar swing. The first click starts the swing, the second click sets the backswing percentage to control power, and the third click sets the impact. The closer the impact is set to the position indicated on the UI the more square the club face will be at impact. Setting the impact point early or late will cause the club face to be more closed or open at impact. This will result in the ball flight to hook or slice in the respected direction

Future mechanics:
Currently the ball placement angle respective to the player is hard coded, future update will include the ability to position the player forward or back along the swing plane which will result an adjustment in the vertical club face angle at impact.
Future update will allow the adjustment of the direction of travel of the club’s swing plan to be from out to in, in to in, and in to out which will result in adjustments to the ball flight path. This combined with the impact face angle will result in all 9 traditional golf flight paths to be simulated.
Power bar tolerance update will penalize very early or late impacts with “skull” and “duff” ball flight. This will be modeled on the bottom of the club impacting the ball or turf dug up by a divot impacting the ball. 
