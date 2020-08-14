# e-cribbage
SMT LED Cribbage Board
![3D ](https://github.com/tchristle/e-cribbage/blob/master/KiCAD/3D_image.png)

**To-Do List**
**-----------------------------------------------------------------**
* Schematic
	- [x] assign different symbols to P1 and P2 LEDs
	- [x] assign correct parts numbers to switch and CR2032 symbols
	- [ ] assign final values to LED current limiting resistor
	- [ ] add test points for debug
* Layout
	- [x] modify CR2032 footprint to match selected clip
	- [x] modify B5 footprint to match selected slide switch
	- [x] assign common footprint to all capacitors
	- [x] update footprint for S1-S4 tactile push buttons
	- [x] generate gerbers and drill
* BOM
	- [x] generate BOM from KiCAD schematic
	- [ ] attach Digikey PNs to BOM
	- [ ] down select LEDs and colors
	- [x] down select slide switch
	- [x] down select tactile push button
	- [ ] generate Digikey cart from BOM
	- [x] get pricing on PCB
	- [x] get pricing on stencils
	- [ ] get pricing on pick and place assembly
* Software
	- [ ] debug flash store and recall functions
	- [ ] replace for loops and counters with timers in LPM0
	- [ ] implement peg swap timer
	- [ ] implement Elapsed Time Indicator
	- [ ] implement Battery Monitor
* Prototype
	- [x] drive sample LEDs with limit resistor to set brightness level
	- [ ] analyze battery life with CR2032 cell
	- [x] interface ez430 with single shifter register
