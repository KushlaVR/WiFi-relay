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
                $(".switch-img").click(function () {
                    let cb = $(".switch-checkbox", $(this).closest(".card"));
                    if (cb.data("state") == "ON") {
                        Relay.relay.turnOff(cb.data("switch"));
                    } else {
                        Relay.relay.turnOn(cb.data("switch"));
                    }
                });
                $(".switch-checkbox").change(function () {
                    if (this.checked) {
                        Relay.relay.turnOn($(this).data("switch"))
                    } else {
                        Relay.relay.turnOff($(this).data("switch"))
                    }
                });
                TimeConverter.Convert();
            }
        ).fail(function (data: any, status: any) {
            $(".process-list").html("Не вдалось завантажити. <button class'btn btn-primary refresh'>Повторити</button>");
            $(".switch-checkbox").click(function () {
                Relay.relay.loadProcesses();
            });
        }
        );
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
            let t = Relay.relay.getTemplate("newitem");
            if (t === undefined) {
                allReady = false;
                Relay.relay.loadTemplateByName("newitem", Relay.relay.loadTriggerTemplate);
                return;
            }
        }

        if (allReady === true) {
            let list: string = "";

            list += Relay.relay.getTemplate("newitem");

            for (let i: number = 0; i < Relay.relay.triggers.length; i++) {
                let item: Trigger = Relay.relay.triggers[i];
                let t: string = Relay.relay.getTemplate(item.template);
                list += "<div class='holder' id='trigger_" + item.uid + "' data-index='" + i.toString() + "' data-uid='" + item.uid + "'>" + Relay.relay.fillTemplate(t, item) + "</div>";
            }

            $(".process-list").html(list);
            $(".btn-edit").click((e) => {
                Relay.relay.edit(e);
            });
            $(".btn-add").click((e) => {
                Relay.relay.add(e);
            });
            $(".switch-checkbox[data-state='on']").prop("checked", true);
            let cp: any = $('.clockpicker');
            cp.clockpicker();
            TimeConverter.Convert();
        }
    }

    public add(e: any): void {
        console.log("add");

        let item: Trigger = $(e.target).data("newitem");
        let t: string = this.getTemplate(item.editingtemplate);
        if (t === undefined) {
            Relay.relay.loadTemplateByName(item.editingtemplate, () => {
                Relay.relay.add(e);
            });
            return;
        }

        var iDiv = document.createElement('div');
        iDiv.id = 'trigger_0';
        iDiv.className = 'holder';
        $(".process-list")[0].appendChild(iDiv);
        let holder: JQuery = $(iDiv)
        holder.data("uid", 0);
        holder.data("edit", true);

        holder.html(this.fillTemplate(t, item));


        $('.btn-edit').attr("disabled", "disabled");
        $('.btn-save').click((e) => {
            Relay.relay.save(e);
        });

        $(".switch-checkbox[data-state='on']").prop("checked", true);
        let cp: any = $('.clockpicker');
        cp.clockpicker();
        TimeConverter.Convert();
    }

    public edit(e: any): void {
        console.log("edit");
        let holder: JQuery = $(e.target).closest(".holder");
        holder.data("edit", true);
        let i: number = holder.data("index");
        let item: Trigger = this.triggers[i];
        let t: string = this.getTemplate(item.editingtemplate);
        holder.html(this.fillTemplate(t, item));


        $('.btn-edit').attr("disabled", "disabled");
        $('.btn-save').click((e) => {
            Relay.relay.save(e);
        });

        $(".switch-checkbox[data-state='on']").prop("checked", true);
        let cp: any = $('.clockpicker');
        cp.clockpicker();
        TimeConverter.Convert();
    }

    public save(e: any): void {
        console.log("save");
        let holder: JQuery = $(e.target).closest(".holder");
        let form: JQuery = $("form", holder);
        let fields = $("[name]", form);
        let url: string = Relay.relay.rooturl + "setup?type=save&switch=" + this.getUrlParameter("index") + "&uid=" + holder.data("uid");
        for (let i: number = 0; i < fields.length; i++) {
            let v: string;
            let f = $(fields[i]);
            if (f.hasClass("converter-time")) v = TimeConverter.ConvertBack(fields[i]); else v = TextConverter.ConvertBack(fields[i]);
            url += "&" + fields[i].getAttribute("name") + "=" + encodeURIComponent(v);
        }
        $.get(url).done(function (data: any, status: any) {
            location.reload();
        }
        ).fail(function (data: any, status: any) {
            alert(data);
        }
        );
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
                $("#switch_" + i.toString()).prop("checked", true);
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
                $("#switch_" + i.toString()).prop("checked", false);
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

class TimeConverter {


    public static Convert(): void {

        let inputs = $(".converter-time")
            .each((index: number, elem: Element): void => {
                let el = $(elem);
                if (el.data("converted") === "1") return;

                let v: string;
                if (elem.tagName === "INPUT") {
                    v = el.val();
                } else {
                    v = elem.innerHTML;
                }

                let res: string = "";
                let i: number = parseInt(v, 10);
                res = TimeConverter.pad((Math.floor(i / 60)).toString(), 2) + ":" + TimeConverter.pad((i % 60).toString(), 2);
                el.data("converted", "1");
                if (elem.tagName === "INPUT") {
                    el.attr("value", res);
                } else {
                    elem.innerHTML = res;
                }
            });
    }

    public static ConvertBack(elem: Element): string {
        let el = $(elem);
        if (el.data("converted") === "1") {
            let v: string;
            if (elem.tagName === "INPUT") {
                v = el.val();
            } else {
                v = elem.innerHTML;
            }
            let parts = v.split(":");
            let res: number = parseInt(parts[0]) * 60 + parseInt(parts[1]);
            return res.toString();
        }
        return el.val();
    }


    public static pad(value: string, size: number): string {
        var s = value;
        while (s.length < (size || 2)) { s = "0" + s; }
        return s;
    }

}



class TextConverter {


    public static ConvertBack(elem: Element): string {
        let el = $(elem);
        if (el.attr("type") === "checkbox") {
            return el.prop("checked");
        }
        return el.val();
    }

}