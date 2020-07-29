# e-cribbage
SMT LED Cribbage Board
![3D ](https://github.com/tchristle/e-cribbage/blob/master/KiCAD/3D_image.png)

**To-Do List**
**-----------------------------------------------------------------**
* Schematic
	* assign different symbols to P1 and P2 LEDs
	* assign correct parts numbers to switch and CR2032 symbols
	* assign final values to LED current limiting resistor
	* add test points for debug
* Layout
	* modify CR2032 footprint to match selected clip
	* modify B5 footprint to match selected slide switch
	* assign common footprint to all capacitors
	* update footprint for S1-S4 tactile push buttons
	* generate gerbers and drill
* BOM
	* generate BOM from KiCAD schematic
	* attach Digikey PNs to BOM
	* down select LEDs and colors
	* down select slide switch
	* down select tactile push button
	* generate Digikey cart from BOM
	* get pricing on PCB
	* get pricing on stencils
	* get pricing on pick and place assembly
* Software
	* debug flash store and recall functions
	* replace for loops and counters with timers in LPM0
	* implement peg swap timer
* Prototype
	* drive sample LEDs with limit resistor to set brightness level
	* analyze battery life with CR2032 cell
	* interface ez430 with single shifter register
