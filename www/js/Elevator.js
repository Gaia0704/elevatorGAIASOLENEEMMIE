const MAX_PASSENGERS = 5
 
export default class Building {
    #doors    = []
    #floors
    #cabin
    #position
    #columnHeight
    #columnOffsets
    #columnCount
    #floorsByColumn
    #floorCount
    #floorHeight
    #cabinOffsetX
    #cabinOffsetY
    #cabinPassageState = null
    #buildingHeight
    #analog
    #buttons
 
    constructor(selectors) {
        this.#floors = selectors.floors.map(sel => 
            document.querySelector(sel) 
            || console.warn(`Floor '${sel}' not found.`)
        )        
        this.#analog  = this.#selectors2elements(selectors.analog)
        this.#buttons = this.#selectors2elements(selectors.buttons)
        this.#floorHeight = this.#floors[0].offsetHeight
        this.#cabin = document.querySelector(selectors.cabin)
        this.#cabinOffsetX = this.#cabin.offsetLeft
        this.#cabinOffsetY = this.#floorHeight - this.#cabin.offsetHeight - this.#cabin.offsetTop
        this.#position = this.#cabin.offsetHeight + this.#floorHeight
 
        // Force une seule colonne pour éviter le bug de disposition
        this.#columnOffsets   = [ this.#floors[0].offsetLeft ]
        this.#columnCount     = 1
        this.#floorCount      = this.#floors.length
        this.#floorsByColumn  = this.#floorCount
        this.#columnHeight    = this.#floorsByColumn * this.#cabin.offsetHeight 
                              + this.#floorHeight * (this.#floorsByColumn - 1)
        this.#buildingHeight  = this.#columnHeight - this.#cabin.offsetHeight
 
        for(let i in this.#floors) {
            const right = this.#floors[i].querySelector(selectors.doors[1])
            const width = right.offsetWidth
            const door = {
                center: right.offsetLeft,
                width,
                left : this.#floors[i].querySelector(selectors.doors[0]), 
                right: right,
                pos: i == this.#currentFloor() ? width : 0
            }
            this.#doors.push(door)
            this.#updateDoors(door)
        }
    }
 
    #between = (n, min, max) => min <= n && n < max
 
    #updateDoors = (door) => {
        door.left.style.left  = `${door.center - door.pos - door.width}px`
        door.right.style.left = `${door.center + door.pos}px`
    }
 
    #selectors2elements(selectors) {
        return Object.fromEntries(Object.keys(selectors).map(k => {
            const e = document.querySelector(selectors[k])
            if(e) 
                return [ k, e ]
            console.warn(`Element '${selectors[k]}' associated to key '${k}' not found`)
            return [undefined, undefined]
        }))
    }
 
    floorTitle(floor) {
        return this.#buttons[`cabin${floor}`].innerText
    }
    cabin() {
        return this.#cabin
    }
    floorCount() {
        return this.#floorCount
    }
    floor(floorIndex) {
        return this.#floors[floorIndex]
    }
    isCabinFull() {
        return this.#cabin.querySelectorAll('li').length < MAX_PASSENGERS
    }
    beginPass(floor, delta, name, additionalPredicate = undefined) {
        if(this.#cabinPassageState) {
            return false
        }
        const cur = this.#currentFloor()
        const door = this.#doors[cur]
        
        if(floor !== undefined && cur != floor || 
            door.pos < door.width * 2 / 3 || 
            additionalPredicate !== undefined && !additionalPredicate()
        ) return false
        this.analog(delta, name)
        this.#cabinPassageState = { delta: -delta, name: name }
        return true
    }
    endPass() {
        const state = this.#cabinPassageState
        this.analog(state.delta, state.name)
        this.#cabinPassageState = null
    }
    #currentFloor() {
        const step = this.#cabin.offsetHeight + this.#floorHeight
        return Math.floor(this.#position / step)
    }
    elevatorFloor() {
        return this.#currentFloor()
    }
    moveDoors(dir) {
        const floor = this.#currentFloor()
        if(this.#between(floor, 0, this.#floorCount)) {
            const door = this.#doors[floor]
            const nextPos = door.pos - dir
            if(this.#between(nextPos, 0, door.width + 1)) {
                door.pos = nextPos
                this.#updateDoors(door)
            }
        }
    }
    moveElevator(dir) {
        const nextPos = this.#position + dir
        if(this.#between(nextPos, 0, this.#buildingHeight + 1)) {
            const step = this.#cabin.offsetHeight + this.#floorHeight
            const floor = Math.floor(nextPos / step)
            const floorEl = this.#floors[Math.min(floor, this.#floorCount - 1)]
            
            this.#position = nextPos
            this.#cabin.style.top  = `${floorEl.offsetTop + this.#floorHeight - this.#cabin.offsetHeight - this.#cabinOffsetY}px`
            this.#cabin.style.left = `${this.#columnOffsets[0] + this.#cabinOffsetX}px`
        }
    }
    button(pressed, name, floor) {
        const e = new Event(
            pressed ? "mousedown" : "mouseup",
            { bubbles: true, cancelable: true }
        )
        if(floor !== undefined) {
            name = `floor${floor}${name}`
        } 
        this.#buttons[name]?.dispatchEvent(e)
    }
    analog(delta, name) {
        const val = parseInt(this.#analog[name].value) + delta
        this.#analog[name].value = delta < 0 
            ? Math.max(this.#analog[name].min, val) 
            : Math.min(this.#analog[name].max, val)
        this.#analog[name].dispatchEvent(new Event('input'))
    }
}