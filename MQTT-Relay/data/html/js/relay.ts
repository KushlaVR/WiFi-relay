class Relay {

    private rooturl: String = "api/";
    public static relay: Relay = new Relay();

    public init(): void {
        let setup: any = this.getUrlParameter("setup");
        if (setup === undefined) {
            $("#pageHeader").html("Стан виходів");
            this.loadTemplateByName("switch", this.loadProcesses);
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
                        let t: string = Relay.relay.getTemplate(item.visual);
                        if (t === undefined) {
                            list += "<li class='nav-item'>";
                            list += item.name;
                            list += "</li>";
                        } else {
                            list += Relay.relay.fillTemplate(t, item);
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

    public loadTemplateByName(name: string, onDone: any): void {

        $.get(Relay.relay.rooturl + "template?name=" + name).done(
            function (data: any, status: any) {
                let item: keyValue = new keyValue();
                item.key = name;
                item.value = data;
                Relay.relay.templates.push(item);
                onDone();
            }
        ).fail(function (data: any, status: any) {
            $(".process-list").html("Не вдалось завантажити. <button class'btn btn-primary refresh'>Повторити</button>");
            $(".switch-checkbox").click(function () {
                Relay.relay.loadTriggerTemplate();
            });
        }
        );

    }

    public loadTriggerTemplate(): void {
        let allReady: boolean = true;
        for (let i: number = 0; i < Relay.relay.triggers.length; i++) {
            let item: Trigger = Relay.relay.triggers[i];
            let t: string = Relay.relay.getTemplate(item.template);
            if (t === undefined) {
                allReady = false;
                Relay.relay.loadTemplateByName(item.template, Relay.relay.loadTriggerTemplate);
                return;
            }

            t = Relay.relay.getTemplate(item.editingtemplate);
            if (t === undefined) {
                allReady = false;
                Relay.relay.loadTemplateByName(item.editingtemplate, Relay.relay.loadTriggerTemplate);
                return;
            }
        };
        if (allReady === true) {
            let list: string = "";

            for (let i: number = 0; i < Relay.relay.triggers.length; i++) {
                let item: Trigger = Relay.relay.triggers[i];
                let t: string = Relay.relay.getTemplate(item.template);
                list += "<div style='display:inline-block;' class='holder' id='trigger_" + item.uid + "' data-index='" + i.toString() + "'>" + Relay.relay.fillTemplate(t, item) + "</div>";
            }

            $(".process-list").html(list);
            $(".btn-edit").click((e) => {
                Relay.relay.edit(e);
            });
            $(".switch-checkbox[data-action='on']").prop("checked", true);
        }
    }

    private fillTemplate(t: string, item: Trigger): string {
        let ret: string = t;
        for (var key in item) {
            let s: string = item[key];
            let k: string = "@" + key + "@";
            while (ret.indexOf(k) >= 0) {
                ret = ret.replace(k, s);
            }
        }
        return ret;
    }

    public edit(e: any): void {
        console.log("edit");
        let holder: JQuery = $(e.target).closest(".holder");
        holder.data("edit", true);
        $('.btn-edit').attr("disabled", "disabled");
        let i: number = holder.data("index");
        let item: Trigger = this.triggers[i];
        let t: string = this.getTemplate(item.editingtemplate);
        holder.html(this.fillTemplate(t, item));
        $(".switch-checkbox[data-action='on']").prop("checked", true);
    }

    public loadSetup(setup: any, index: number): void {
        $.get(Relay.relay.rooturl + "setup?type=switch&index=" + index.toString()).done(
            function (data: any, status: any) {
                for (let i: number = 0; i < data.items.length; i++) {
                    let item: Trigger = data.items[i];
                    Relay.relay.triggers.push(item);
                };
                Relay.relay.loadTriggerTemplate();
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
    public uid: string;
    public name: string;
    public desc: string;
    public type: string;
    public action: string;
    public template: string;
    public editingtemplate: string;
}