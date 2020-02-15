export enum DIGITALVALUE {
    LOW = 0,
    HIGH = 1
}

export enum PINMODE {
    INPUT        = 0b001,
    OUTPUT       = 0b010,
    INPUT_PULLUP = 0b101
};

export declare function millis(): u32;
export declare function delay(ms: u32): void;
export declare function pinMode(pin: u32, mode: PINMODE): void;
export declare function digitalWrite(pin: u32, value: DIGITALVALUE): void;

@external("print")
declare function _print(ptr: usize, len: usize): void;

export function print(str: string): void {
    const buffer = String.UTF8.encode(str);

    _print(changetype<usize>(buffer), buffer.byteLength);
}