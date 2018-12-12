var Relay = (function () {
    function Relay() {
        this.rooturl = "api/";
    }
    Relay.prototype.init = function () {
        Relay.relay.loadProcesses();
    };
    Relay.prototype.loadProcesses = function () {
        $.get(Relay.relay.rooturl + "switches").done(function (data, status) {
            var w = data;
            var list = "";
            for (var i = 0; i < w.items.length; i++) {
                var item = w.items[i];
                if (item.visual) {
                    if (item.visual == "switch") {
                        list += "<div class='card' style='width: 18rem;'>";
                        list += "  <img class='card-img-top light-off' src='/content/idea.svg' alt='Вимикач' id='switch_img_" + item.index + "' data-state='" + item.state + "'>";
                        list += "  <div class='card-body'>";
                        list += "    <h5 class='card-title'>Вихід №" + item.index + "</h5>";
                        list += "<label class=" + item.type + ">";
                        list += "<input type='checkbox' class='switch-checkbox' id='switch_" + item.index + "'data-switch='" + item.index + "' data-state='" + item.state + "'/>";
                        list += "<span class='slider'></span>";
                        list += "</label>";
                        list += "  </div>";
                        list += "  <ul class='list-group list-group-flush'>";
                        list += "    <li class='list-group-item'><a href='#' class='card-link'>Графік</a></li>";
                        list += "  </ul>";
                        list += "</div>";
                    }
                    else {
                        list += "<li class='nav-item'>";
                        list += item.name;
                        list += "</li>";
                    }
                }
            }
            ;
            $(".process-list").html(list);
            $(".switch-checkbox[data-state='ON']").prop("checked", true);
            $("img[data-state='ON']").removeClass("light-off");
            $(".switch-checkbox").change(function () {
                if (this.checked) {
                    Relay.relay.turnOn($(this).data("switch"));
                }
                else {
                    Relay.relay.turnOff($(this).data("switch"));
                }
            });
        }).fail(function (data, status) {
            $(".process-list").html("Не вдалось завантажити. <button class'btn btn-primary refresh'>Повторити</button>");
            $(".switch-checkbox").click(function () {
                Relay.relay.loadProcesses();
            });
        });
    };
    Relay.prototype.turnOn = function (i) {
        if ($("#switch_" + i.toString()).data("state") == "ON")
            return;
        console.log("turn on");
        $.post(Relay.relay.rooturl + "switches" + "?index=" + i.toString() + "&state=on").done(function (data, status) {
            $("#switch_" + i.toString()).data("state", "ON");
            $("#switch_img_" + i.toString()).removeClass("light-off").addClass("light-on");
        }).fail(function (data, status) {
            $("#switch_" + i.toString()).prop("checked", false);
        });
        return true;
    };
    Relay.prototype.turnOff = function (i) {
        if ($("#switch_" + i.toString()).data("state") == "OFF")
            return;
        console.log("turn off");
        $.post(Relay.relay.rooturl + "switches" + "?index=" + i.toString() + "&state=off").done(function (data, status) {
            $("#switch_" + i.toString()).data("state", "OFF");
            $("#switch_img_" + i.toString()).removeClass("light-on").addClass("light-off");
        }).fail(function (data, status) {
            $("#switch_" + i.toString()).prop("checked", true);
        });
        return true;
    };
    Relay.prototype.wifi = function () {
        $.get(Relay.relay.rooturl + "wifi").done(function (data, status) {
            var w = data;
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
            $(".wifi-list").html("Не вдалось завантажити список мереж.<br />Поновіть сторінку, щоб повторити спробу.");
        });
    };
    Relay.relay = new Relay();
    return Relay;
}());
var WIFI_list = (function () {
    function WIFI_list() {
    }
    return WIFI_list;
}());
var Items_list = (function () {
    function Items_list() {
    }
    return Items_list;
}());
//# sourceMappingURL=relay.js.map