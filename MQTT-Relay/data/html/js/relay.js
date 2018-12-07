var Relay = /** @class */ (function () {
    function Relay() {
    }
    //public static relay: Relay = new Relay();
    //static Init() {
    //    Relay.relay._init();
    //}
    Relay.prototype._init = function () {
        $(".switch").change(function () {
            if (this.checked) {
                Relay.relay.turnOn($(this).data("switch"));
            }
            else {
                Relay.relay.turnOff($(this).data("switch"));
            }
        });
    };
    Relay.prototype.turnOn = function (i) {
        return true;
    };
    Relay.prototype.turnOff = function (i) {
        return true;
    };
    Relay.relay = new Relay();
    return Relay;
}());
//# sourceMappingURL=relay.js.map