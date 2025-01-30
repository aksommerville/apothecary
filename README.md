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
- - [x] !!!!!! I think we've got scoring backward.
- - - Instead of testing how many deliveries you can make in a fixed time, we should test how much time you take to make a fixed set of deliveries.
- - - Ten deliveries, say? And the order will be random each time. So scores are comparable but you can't do it blindfolded.
- - - What's nice about that is it's not quantized like delivery counts. You score like 2:04.123, and in the future you could edge that out by a fraction of a second.
- - - ...yes a fixed set of deliveries, scored by time, plays much nicer.
- - - With 10 dropoffs, I'm scoring around 2:40.
- - - Reduced to 7 and now 2:00 is doable but difficult.
- Fri 31 Jan
- - [x] Menus.
- - - [x] Hello
- - - [x] Game over
- - [x] Persist high score.
- - [x] Fade out after game ends, then switch to the game over screen. 3 seconds or so?
- - [x] hello: Quit, Play, Settings.
- - [x] Settings: Language, Music, Sound, Zap high score, Configure input.
- - [x] Text for hello. My name, date, mention Heaven Jam...
- - [x] hello: When Dot flies leftward, scale her down a bit, tint a bit to black, and render behind the title.
- - [x] gameover: Don't say "new high score" when it's a tie. New message for that.
- - [x] Physics: Tighten up cornering and braking, and ensure when pressing into a wall, direction still shifts as if you were free -- it doesn't right now, and it's a little weird.
- - [x] Turn off music and sound. UI is present and sets a flag, but doesn't actually do anything.
- - [x] Tune map so there are neat tricks you can pull, like there's always a wall to bounce off where you need one.
- - [x] Show progress 0..6
- - [x] Option to abort game, press Start.
- Sat 1 Feb
- - [ ] I checked with admins and it's kosher: Submit early to the jam. Maybe get some feedback while we're still allowed to update.
- - [x] Music.
- - [x] French and Spanish text, why not. Russian(TurtleMan), Finnish(Joop)
- - [x] Thirty Seconds doesn't loop quite right.
- - [x] Emotional Support Bird: Add arpeggio peak notes to bass in second pass. (right hand does that on a piano, but with different voices one notices a gap).
- - [x] Sound effects.
- - - [x] Braking.
- - - [x] Flying: Ongoing repeating sound, varies with velocity.
- - - [x] Pickup.
- - - [x] Dropoff.
- - [x] 7 distinct customers
- - [ ] Decorative touches.
- - - [ ] Little people that go "hey" and jump out of your way.
- - [x] physics: I glossed over corner collisions. Does it matter? ...been playing it for two days, and no, i don't notice anything wrong.
- - [ ] Egg's subtractive voices are markedly different native vs web. I like what we have for native. Look into repairing egg.
- - [ ] Plan for game to be complete by EOD.
- Sun 2 Feb
- - [ ] Itch page, ancillary bits, final upload.
- - [ ] Submit.
- Mon 3 Feb
- - Jam ends 23:45. Time for last-minute repairs etc. Don't plan anything for today.

Stretch goals.
- [ ] Multiplayer.
- - Shared screen doesn't seem workable, and I don't have much appetite for implementing split screen.
- [ ] Different play modes... Open exploration? Race in a loop?
- [ ] Multiple deliveries at once.
- [ ] Hazards.

## Progress Log

Mon 27 Jan: Flying, map loading, music. Rather more than I expected to get done day one!
Tue 28 Jan: Random pickup and deliver. Clock, score. Preliminary hello and gameover splashes.
Wed 29 Jan: Prettier graphics for Dot. Real map. Changed to time-based scoring with fixed deliveries. Hello menu.
Thu 30 Jan: Sound effects.
Fri 31 Jan: 
Sat 1 Feb: 
Sun 2 Feb: 
Mon 3 Feb: 
