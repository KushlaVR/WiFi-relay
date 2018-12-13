
class Relay {

    private rooturl: String = "api/";
    public static relay: Relay = new Relay();

    public init(): void {
        let setup: any = this.getUrlParameter("setup");
        if (setup === undefined) {
            $("#pageHeader").html("Стан виходів");
            this.loadProcesses();
        } else {
            $("#pageHeader").html("Налаштування");
            this.loadSetup(setup, this.getUrlParameter("index"));
        }
    }

    public loadProcesses(): void {
        $.get(Relay.relay.rooturl + "switches").done(
            function (data: any, status: any) {
                let w: Items_list = data;
                let list: string = "";
                for (let i: number = 0; i < w.items.length; i++) {
                    let item: any = w.items[i];
                    if (item.visual) {
                        if (item.visual == "switch") {
                            list += "<div class='card m-3' style='width: 18rem; display:inline-block;'>"
                            list += "  <img class='card-img-top light-off' src='/content/idea.svg' alt='Вимикач' id='switch_img_" + item.index + "' data-state='" + item.state + "'>"
                            list += "  <div class='card-body'>"
                            list += "    <h5 class='card-title'>Вихід №" + item.index + "</h5>"

                            list += "<label class=" + item.type + ">";
                            list += "<input type='checkbox' class='switch-checkbox' id='switch_" + item.index + "'data-switch='" + item.index + "' data-state='" + item.state + "'/>";
                            list += "<span class='slider'></span>";
                            list += "</label>";

                            list += "  </div>"
                            list += "  <ul class='list-group list-group-flush'>"
                            list += "    <li class='list-group-item'><a href='/index.html?setup=switch&index=" + item.index + "' class='card-link'>Налаштувати</a></li>"
                            list += "  </ul>"
                            list += "</div>"

                        } else {
                            list += "<li class='nav-item'>";
                            list += item.name;
                            list += "</li>";
                        }
                    }
                };
                $(".process-list").html(list);
                $(".switch-checkbox[data-state='ON']").prop("checked", true);
                $("img[data-state='ON']").removeClass("light-off");
                $(".switch-checkbox").change(function () {
                    if (this.checked) {
                        Relay.relay.turnOn($(this).data("switch"))
                    } else {
                        Relay.relay.turnOff($(this).data("switch"))
                    }
                });
            }
        ).fail(function (data: any, status: any) {
            $(".process-list").html("Не вдалось завантажити. <button class'btn btn-primary refresh'>Повторити</button>");
            $(".switch-checkbox").click(function () {
                Relay.relay.loadProcesses();
            });
        }
        );
    }

    private templates: Array<keyValue> = new Array<keyValue>();
    public triggers: Array<Trigger> = new Array<Trigger>();

    private getTemplate(s: string): string {
        for (let i: number = 0; i < this.templates.length; i++) {
            let item: keyValue = this.templates[i];
            if (item.key === s) {
                return item.value;
            }
        };
        return undefined;
    }

    public loadTemplateByName(name: string): void {

        $.get(Relay.relay.rooturl + "template?name=" + name).done(
            function (data: any, status: any) {
                let item: keyValue = new keyValue();
                item.key = data.name;
                item.value = data.html;
                Relay.relay.templates.push(item)
                Relay.relay.loadTemplate();
            }
        ).fail(function (data: any, status: any) {
            $(".process-list").html("Не вдалось завантажити. <button class'btn btn-primary refresh'>Повторити</button>");
            $(".switch-checkbox").click(function () {
                Relay.relay.loadTemplate();
            });
        }
        );

    }

    public loadTemplate(): void {
        let allReady: boolean = true;
        for (let i: number = 0; i < this.triggers.length; i++) {
            let item: Trigger = this.triggers[i];
            let t: string = this.getTemplate(item.template);
            if (t === undefined) {
                allReady = false;
                this.loadTemplateByName(item.template);
            }

            t = this.getTemplate(item.editingtemplate);
            if (t === undefined) {
                allReady = false;
                this.loadTemplateByName(item.editingtemplate);
            }
        };
        if (allReady === true) {
            let list: string = "";

            for (let i: number = 0; i < this.triggers.length; i++) {
                let item: Trigger = this.triggers[i];
                let t: string = this.getTemplate(item.template);
                list += this.fillTemplate(t, item);
            }

            $(".process-list").html(list);
            $(".btn-edit").click((e) => {
                Relay.relay.edit(this);
            })
        }
    }

    private fillTemplate(t: string, item: Trigger): string {
        return t.replace("#name#", item.name)
            .replace("#action#", item.action)
            .replace("#type#", item.type)
            .replace("#desc#", item.desc)
    }

    public edit(e:any): void {
        console.log("edit");

    }

    public loadSetup(setup: any, index: number): void {
        $.get(Relay.relay.rooturl + "setup?type=switch&index=" + index.toString()).done(
            function (data: any, status: any) {
                for (let i: number = 0; i < data.items.length; i++) {
                    let item: Trigger = data.items[i];
                    Relay.relay.triggers.push(item);
                };
                Relay.relay.loadTemplate();
            }
        ).fail(function (data: any, status: any) {
            $(".process-list").html("Не вдалось завантажити. <button class'btn btn-primary refresh'>Повторити</button>");
            $(".switch-checkbox").click(function () {
                Relay.relay.loadSetup(setup, index);
            });
        }
        );
    }

    public getUrlParameter(sParam): any {
        let sPageURL: string = window.location.search.substring(1);
        let sURLVariables: string[] = sPageURL.split('&');
        let sParameterName: string[];
        let i: number;

        for (i = 0; i < sURLVariables.length; i++) {
            sParameterName = sURLVariables[i].split('=');
            if (sParameterName[0] === sParam) {
                return (sParameterName[1] === undefined) ? true : decodeURIComponent(sParameterName[1]);
            }
        }
        return undefined;
    }

    public turnOn(i: number): boolean {
        if ($("#switch_" + i.toString()).data("state") == "ON") return;

        console.log("turn on");
        $.post(Relay.relay.rooturl + "switches" + "?index=" + i.toString() + "&state=on").done(
            function (data: any, status: any) {
                $("#switch_" + i.toString()).data("state", "ON");
                $("#switch_img_" + i.toString()).removeClass("light-off").addClass("light-on")
            }
        ).fail(
            function (data: any, status: any) {
                $("#switch_" + i.toString()).prop("checked", false);
            }
        );

        return true;
    }

    public turnOff(i: number): boolean {
        if ($("#switch_" + i.toString()).data("state") == "OFF") return;

        console.log("turn off");
        $.post(Relay.relay.rooturl + "switches" + "?index=" + i.toString() + "&state=off").done(
            function (data: any, status: any) {
                $("#switch_" + i.toString()).data("state", "OFF");
                $("#switch_img_" + i.toString()).removeClass("light-on").addClass("light-off")
            }
        ).fail(
            function (data: any, status: any) {
                $("#switch_" + i.toString()).prop("checked", true);
            }
        );
        return true;
    }

    public wifi(): void {
        $.get(Relay.relay.rooturl + "wifi").done(
            function (data: any, status: any) {
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

            }).fail(function (data: any, status: any) {
                //alert("Error: " + data + "\nStatus: " + status);
                $(".wifi-list").html("Не вдалось завантажити список мереж.<br />Поновіть сторінку, щоб повторити спробу.");
            });
    }

}

class WIFI_list {
    public ssid: Array<any>;
}


class Items_list {
    public items: Array<any>;
}

class keyValue {
    public key: string;
    public value: any;
}

class Trigger {
    public name: string;
    public desc: string;
    public type: string;
    public action: string;
    public template: string;
    public editingtemplate: string;
}