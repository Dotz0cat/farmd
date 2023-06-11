# Farmd

A half of a farm game. farmc will be made one day.

farmd controls all the backend of the game and farmc will be how you play it.

## steps for alpha

- [x] plant and harvest work correctly

- [x] start fleshing out trees

- [x] framework built for small processing
	- Satisfied by the addition of timestamps and the box_for_list_and_db struct

- [x] config working correctly

- [x] skill tree started well

- [x] buy and sell working

- [x] skill points added

- [x] post arguments taken correctly

- [x] find a standard to document api and api quirks
	- mostly the readme at this stage

- [x] https added

- [x] memory cleaned up

## Steps for beta

- [ ] processing added
	- [ ] dairy
	- [ ] feedmill
	- [ ] grainmill
	- [ ] sugarmill

- [x] building upgrades

- [ ] livestock

- [x] ping save

- [ ] basic automation
	- [ ] auto harvest field
	- [ ] auto plant fields
	- [ ] auto harvest trees

- [x] tree maturity

- [ ] improve functions
	- candidates
		- [ ] get_product_type_string
		- [ ] get_storage_type_string
		- [ ] field_crop_string_to_enum
		- [ ] tree_crop_string_to_enum
		- [ ] skill_dep_check

- [x] special item system
	- started

- [ ] variable xp

- [ ] add times/prices for everything

- [ ] find a standard to document api and api quirks

- [ ] todo moved away from readme

## Steps for 1.0

- [ ] works

- [ ] optimzation

- [ ] no memory leaks
	- [ ] asan clean
	- [ ] valgrind clean

- [ ] full man pages

- [ ] proper readme

## API documentation

It will run at over http on localhost port 8080. (port will be configurable)
And over https over localhost port 8443 (when enabled and given correct keys)

- /barnQuery
- /barn/query
	- Query the barn for an item
	- Uses http GET
	- Item passed as query
	- example 
		- `$ curl http://localhost:8080/barnQuery?apples -X GET`

- /siloQuery
- /silo/query
	- Query the silo for an item
	- Uses http GET
	- Item passed as query
	- example 
		- `$ curl http://localhost:8080/siloQuery?wheat -X GET`

- /createSave
	- Create a game save (sql database) at the location
	- Uses http POST
	- Location can be passed as query or POST arguments
	- example
		- `$ curl http://localhost:8080/createSave?/home/dotz/farmd_save.db -X POST`

- /openSave
	- Opens a game save
	- Uses http POST
	- Location can be passed as query or POST arguments
	- example
		- `$ curl http://localhost:8080/openSave?/home/dotz/farmd_save.db -X POST`

- /closeSave
	- Closes the currently open save
	- Uses http POST
	- example
		- `$ curl http://localhost:8080/closeSave -X POST`

- /barnAllocation
- /barn/allocation
	- Gives the current allocation of the barn in percent round to 2 decimals
	- Uses http GET
	- example
		- `$ curl http://localhost:8080/barnAllocation -X GET`

- /siloAllocation
- /silo/allocation
	- Gives the current allocation of the silo in percent round to 2 decimals
	- Uses http GET
	- example
		- `$ curl http://localhost:8080/siloAllocation -X GET`

- /getMoney
	- Get the current amount of money
	- Uses http GET
	- example
		- `$ curl http://localhost:8080/getMoney -X GET`

- /getLevel
	- Get the current level
	- Uses http GET
	- example
		- `$ curl http://localhost:8080/getLevel -X GET`

- /getXp
	- Get the current amount of xp
	- Uses http GET
	- example
		- `$ curl http://localhost:8080/getXp -X GET`

- /getSkillPoints
	- Get the current amount of skill points
	- Uses http GET
	- example
		- `$ curl http://localhost:8080/getSkillPoints -X GET`

- /getSkillStatus
	- Get the current status of a skill
	- Uses http GET
	- Takes skill name as query
	- example
		- `$ curl http://localhost:8080/getSkillStatus?Farming -X GET`

- /version
	- Shows the current version of farmd
	- Uses http GET
	- example
		- `$ curl http://localhost:8080/version -X GET`

- /getBarnMax
- /barn/max
	- Gets the max compacity of the barn
	- Uses http GET
	- example
		- `$ curl http://localhost:8080/barn/max -X GET`

- /getSiloMax
- /silo/max
	- Gets the max compacity of the silo
	- Uses http GET
	- example
		- `$ curl http://localhost:8080/silo/max -X GET`

- /field/plant
	- Plant all the fields with sent crop
	- Uses http POST
	- Takes crop to plant as query or post argument
	- example
		- `$ curl http://localhost:8080/field/plant?wheat -X POST`

- /field/harvest
	- Harvest the fields that are ready
	- Uses http POST
	- example
		- `$ curl http://localhost:8080/field/harvest -X POST`

- /field/status
	- Shows the current status of the field (ready to harvest or not)
	- Uses http GET
	- example
		- `$ curl http://localhost:8080/field/status -X GET`

- /field/buy
- /buy/field
	- Buy a new field
	- Uses http POST
	- Buys one at a time
	- example
		- `$ curl http://localhost:8080/buy/field -X POST`

- /tree/buy
- /buy/tree
	- Buy a new tree plot
	- Uses http POST
	- Buys one at a time
	- example
		- `$ curl http://localhost:8080/buy/tree -X POST`

- /skill/buy
- /buy/skill
	- Buy a new field
	- Uses http POST
	- Takes skill name as query or post argument
	- example
		- `$ curl http://localhost:8080/buy/skill?Farming -X POST`

- /tree/plant
	- Plants a tree in the first open tree plot
	- Uses http POST
	- Takes tree type as query or post argument
	- example
		- `$ curl http://localhost:8080/tree/plant?pears -X POST`

- /tree/harvest
	- Harvests curently ready to harvest trees
	- Uses http POST
	- example
		- `$ curl http://localhost:8080/tree/harvest -X POST`

- /tree/status
	- Shows the status of tree plots
	- Uses http GET
	- example
		- `$ curl http://localhost:8080/tree/status -X GET`

- /buy/item
	- Buys an item
	- Uses http POST
	- Takes item as query or post argument
	- example
		- `$ curl http://localhost:8080/buy/item?wheat -X POST`

- /sell/item
	- Sells an item
	- Uses http POST
	- Takes item as query or post argument
	- example
		- `$ curl http://localhost:8080/sell/item?wheat -X POST`

- /buy/price
	- Shows the price of an item
	- Uses http GET
	- Takes item as query
	- example
		- `$ curl http://localhost:8080/buy/price?wheat -X GET`

- /sell/price
	- Shows the price of an item
	- Uses http GET
	- Takes item as query
	- example
		- `$ curl http://localhost:8080/sell/price?wheat -X GET`

## Skill Tree

The game features a skill tree

- Farming
	- Fields
		- wheat
		- corn
		- potatos
		- beets
		- sugarcane
		- turnips
		- tomatos
		- cucumbers
		- okra
	- TreePlots
		- pears
		- apples
		- oranges
		- peaches
	- LiveStock

- Processing
	- Dairy
		- pasteurized
		- butter
		- cheese
	- FeedMill
		- cowFeed
		- chickenFeed
	- Milling
		- GrainMill
			- wheatFlour
			- cornMeal
		- SugarMill
			- beetSugar
			- caneSugar

## Contributing

Any contibutions are welcome. 

Feel free to open issues or to fork it.

## License

The project is under the GNU GPLv3 or later. A copy of the license is included in COPYING
