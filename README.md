# e-cribbage
SMT LED Cribbage Board
![3D ](https://github.com/tchristle/e-cribbage/blob/master/KiCAD/3D_image.png)

**To-Do List**
**-----------------------------------------------------------------**
* Schematic
	- [x] assign different symbols to P1 and P2 LEDs
	- [x] assign correct parts numbers to switch and CR2032 symbols
	- [ ] assign final values to LED current limiting resistor
* Layout
	- [x] modify CR2032 footprint to match selected clip
	- [x] modify B5 footprint to match selected slide switch
	- [x] assign common footprint to all capacitors
	- [x] update footprint for S1-S4 tactile push buttons
	- [x] generate gerbers and drill
	- [ ] fix up silk screen on foot prints
* Mechanical
	- [ ] design acrylic cover
	- [ ] update view showing current buttons
* BOM
	- [x] generate BOM from KiCAD schematic
	- [ ] attach Digikey PNs to BOM
	- [X] down select LEDs and colors
	- [x] down select slide switch
	- [x] down select tactile push button
	- [ ] generate Digikey cart from BOM
	- [x] get pricing on PCB
	- [x] get pricing on stencils
	- [ ] get pricing on pick and place assembly
* Software
	- [ ] debug flash store and recall functions
	- [ ] replace for loops and counters with timers in LPM0
	- [X] implement peg swap timer
	- [X] implement Elapsed Time Indicator
* Prototype
	- [x] drive sample LEDs with limit resistor to set brightness level
	- [X] analyze battery life with CR2032 cell
	- [x] interface ez430 with single shifter register
