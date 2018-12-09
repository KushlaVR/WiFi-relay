var Relay = /** @class */ (function () {
    function Relay() {
        this.rooturl = "api/";
    }
    Relay.prototype.init = function () {
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
        console.log("turn on");
        return true;
    };
    Relay.prototype.turnOff = function (i) {
        console.log("turn off");
        return true;
    };
    Relay.prototype.wifi = function () {
        $.get(Relay.relay.rooturl + "wifi").done(function (data, status) {
            var w = data;
            //alert("Done: " + data + "\nStatus: " + status);
            var list = "";
            for (var i = 0; i < w.ssid.length; i++) {
                list += "<a class='wifi-item list-group-item' href='#'>";
                list += "<div class='name'>";
                list += w.ssid[i].name;
                list += "</div>";
                list += "<div class='encryption'>";
                if (w.ssid[i].encryption == "7")
                    list += "-";
                else
                    list += "***";
                list += "</div>";
                list += "<div class='rssi'>";
                list += w.ssid[i].rssi;
                list += "</div>";
                list += "</a>";
            }
            ;
            $(".wifi-list").html(list);
            $(".wifi-item").on("click", function () {
                $('#ssid').val($(".name", this).text());
                $('#password').focus();
            });
        }).fail(function (data, status) {
            //alert("Error: " + data + "\nStatus: " + status);
            $(".wifi-list").html("Не вдалось завантажити список мереж.<br />Поновіть сторінку, щоб повторити спробу.");
        });
    };
    Relay.relay = new Relay();
    return Relay;
}());
var WIFI_list = /** @class */ (function () {
    function WIFI_list() {
    }
    return WIFI_list;
}());
var WIFI_item = /** @class */ (function () {
    function WIFI_item() {
    }
    return WIFI_item;
}());
//# sourceMappingURL=relay.js.map