import * as device from "./device";

const LED = 2;

export function _setup(): void {
  device.pinMode(LED, device.PINMODE.OUTPUT);
  device.print("setup"); // <- Works without this line
}

export function _loop(): void {
  device.digitalWrite(LED, device.DIGITALVALUE.HIGH);
  device.delay(500);
  device.digitalWrite(LED, device.DIGITALVALUE.LOW);
  device.delay(900);
}