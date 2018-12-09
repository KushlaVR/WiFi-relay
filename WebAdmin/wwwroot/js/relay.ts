
class Relay {

    private rooturl: String = "api/";
    public static relay: Relay = new Relay();

    public init(): void {

        $(".switch").change(function () {
            if (this.checked) {
                Relay.relay.turnOn($(this).data("switch"))
            } else {
                Relay.relay.turnOff($(this).data("switch"))
            }
        });
    }

    public turnOn(i: number): boolean {
        console.log("turn on");
        return true;
    }

    public turnOff(i: number): boolean {
        console.log("turn off");
        return true;
    }

    public wifi(): void {
        $.get(Relay.relay.rooturl + "wifi").done(
            function (data: any, status) {
                let w: WIFI_list = data;
                //alert("Done: " + data + "\nStatus: " + status);
                let list: string = "";
                for (let i: number = 0; i < w.ssid.length; i++) {
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
                };
                $(".wifi-list").html(list);
                $(".wifi-item").on("click", function () {
                    $('#ssid').val($(".name", this).text());
                    $('#password').focus();
                });

            }).fail(function (data, status) {
                //alert("Error: " + data + "\nStatus: " + status);
                $(".wifi-list").html("Не вдалось завантажити список мереж.<br />Поновіть сторінку, щоб повторити спробу.");
            });
    }

}

class WIFI_list {
    public ssid: Array<any>;
}

class WIFI_item {
    public name: string;

}