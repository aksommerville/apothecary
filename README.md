# Thirty Second Apothecary

Dot Vine delivers medicine by riding her broom through the city.

Requires [Egg](https://github.com/aksommerville/egg) to build.

For [HEAVENJAM #1](https://itch.io/jam/heavenjam-1), 2025-01-27 .. 2025-02-03.

## Plan

- Orthgraphic racing, think Micro Machines.
- Pick up from the apothecary, then they tell you where to deliver it.
- Pick up and delivery both give you some boost time, a decreasing amount each time.
- Play until you run out of time.

## TODO

- Mon 27 Jan
- - [x] Hero render and motion.
- - [x] Scrolling world.
- Tue 28 Jan
- - [x] Map format and loading.
- Wed 29 Jan
- - [x] Physics. Get motion and collisions feeling good.
- - [x] Proper graphics for Dot.
- - [x] World layout.
- - [x] Generate sbox for the world edges, then we don't have to treat them special.
- Thu 30 Jan
- - [x] Deliveries.
- - [x] Clock.
- - [x] Start, pickup, and dropoff positions per map.
- - [x] Visual indicator at pickup and dropoff. Report score and time bonus.
- - [x] Make a couple different building patterns, then populate east side.
- Fri 31 Jan
- - [x] Menus.
- - - [x] Hello
- - - [x] Game over
- - [x] Persist high score.
- - [x] Fade out after game ends, then switch to the game over screen. 3 seconds or so?
- - [ ] hello: Quit, Play, Settings.
- - [ ] Settings: Language, Music, Sound, Zap high score, Configure input.
- - [x] Text for hello. My name, date, mention Heaven Jam...
- - [x] hello: When Dot flies leftward, scale her down a bit, tint a bit to black, and render behind the title.
- - [ ] gameover: Don't say "new high score" when it's a tie. New message for that.
- - [x] Physics: Tighten up cornering and braking, and ensure when pressing into a wall, direction still shifts as if you were free -- it doesn't right now, and it's a little weird.
- Sat 1 Feb
- - [x] Music.
- - [ ] French and Spanish text, why not. Russian(TurtleMan), Finnish(Joop)
- - [ ] Thirty Seconds doesn't loop quite right.
- - [ ] Emotional Support Bird: Add arpeggio peak notes to bass in second pass. (right hand does that on a piano, but with different voices one notices a gap).
- - [ ] Sound effects.
- - - [ ] Braking.
- - - [ ] Flying: Ongoing repeating sound, varies with velocity.
- - - [ ] Pickup.
- - - [ ] Dropoff.
- - [ ] Decorative touches.
- - - [ ] Little people that go "hey" and jump out of your way.
- - [ ] physics: I glossed over corner collisions. Does it matter?
- - [ ] Plan for game to be complete by EOD.
- Sun 2 Feb
- - [ ] Itch page, ancillary bits, final upload.
- - [ ] Submit.
- Mon 3 Feb
- - Jam ends 23:45. Time for last-minute repairs etc. Don't plan anything for today.

Stretch goals.
- [ ] Multiplayer.
- [ ] Different play modes... Open exploration? Race in a loop?
- [ ] Multiple deliveries at once.
- [ ] Hazards.

## Progress Log

Mon 27 Jan: Flying, map loading, music. Rather more than I expected to get done day one!
Tue 28 Jan: Random pickup and deliver. Clock, score. Preliminary hello and gameover splashes.
Wed 29 Jan: Prettier graphics for Dot. Real map.
Thu 30 Jan: 
Fri 31 Jan: 
Sat 1 Feb: 
Sun 2 Feb: 
Mon 3 Feb: 
