
class Relay {

    public static relay: Relay = new Relay();

    //public static relay: Relay = new Relay();

    //static Init() {
    //    Relay.relay._init();
    //}

    private _init(): void {

        $(".switch").change(function () {
            if (this.checked) {
                Relay.relay.turnOn($(this).data("switch"))
            } else {
                Relay.relay.turnOff($(this).data("switch"))
            }
        });
    }

    public turnOn(i: number): boolean {
        return true;
    }

    public turnOff(i: number): boolean {
        return true;
    }

}

