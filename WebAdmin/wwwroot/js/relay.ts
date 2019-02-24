class WebUI {
    public static rooturl: String = "api/";
    static questionTemplate: string;

    public static model: Model;

    public static init(): void {
        let setup: any = Model.getUrlParameter("setup");
        if (setup === undefined) {
            WebUI.model = new HomePage();
        } else {
            WebUI.model = new SetupPage();
        }
        WebUI.model.init();
    }

    public static wifi() {
        WebUI.model = new WifiPage();
        WebUI.model.init();
    }

    public static ask(question: string, yes: any, no: any = null) {
        let questionModal = $("#questionModal");
        questionModal.html(WebUI.questionTemplate);

        let modaTitle = $(".modal-title", questionModal);
        let modalText = $(".modal-text", questionModal);
        let btnYes = $(".btn-yes", questionModal);
        let btnNo = $(".btn-no", questionModal);
        let btnCancel = $(".btn-cancel", questionModal);

        modalText.html(question);
        questionModal.addClass("in").attr("style", "display:block;opacity:1");
        btnYes.click(() => {
            yes();
        });
        btnNo.click(() => {
            if (no) no();
            questionModal.removeClass("in").removeAttr("style");
            return false;
        });
        btnCancel.click(() => {
            questionModal.removeClass("in").removeAttr("style");
            return false;
        });
    }

}

class Model {


    public static getUrlParameter(sParam): any {
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


    private templates: Array<keyValue> = new Array<keyValue>();
    public elements: Array<any> = new Array<any>();


    public init(): void {
        let qm = $("#questionModal");
        if (qm.length > 0)
            WebUI.questionTemplate = $("#questionModal")[0].innerHTML;
        Converters.ClockPicker();
    }

    public fillTemplate(t: string, item: Trigger): string {
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

    public loadFailed(data: any, retyHandler: any) {
        this.updateTime(data);

        $(".process-list").html("Не вдалось завантажити. <button class'btn btn-primary refresh'>Повторити</button>");
        $(".refresh").click(function () {
            if (retyHandler) retyHandler();
        });
    }

    public updateTime(data: any): void {
        let time_str: string = "";
        if (data.systime) { time_str += data.systime; };
        if (data.uptime) {
            let ms: number = data.uptime;
            let uptime: string = "";
            let second: number = Math.floor(ms / 1000) % 60;
            let minute: number = Math.floor(ms / 1000 / 60) % 60;
            let hour: number = Math.floor(ms / 1000 / 60 / 60) % 24;
            let days: number = Math.floor(ms / 1000 / 60 / 60 / 24);
            if (days > 0) uptime += days.toString() + " днів";
            if (days > 0 || hour > 0) uptime += " " + hour.toString() + ":";
            uptime += minute.toString();
            uptime += ":" + second.toString();
            time_str += " (час роблти - " + uptime + ")";
        };
        if (data.systime || data.uptime) $("#systime").html(time_str);
    }

    public loadTemplateByName(name: string, onDone: any): void {
        let _name: string = name;
        $.get(WebUI.rooturl + "template?name=" + name).done(
            (data: any, status: any) => this.templateByNameLoaded(_name, data, onDone)
        ).fail(
            (data: any, status: any) => this.loadFailed(data, this.loadElementsTemplate)
        );
    }

    private templateByNameLoaded(name: string, data: any, onDone: any): void {
        this.updateTime(data);
        let item: keyValue = new keyValue();
        item.key = name;
        item.value = data;
        this.templates.push(item);
        onDone();
    }

    public getTemplate(s: string): string {
        for (let i: number = 0; i < this.templates.length; i++) {
            let item: keyValue = this.templates[i];
            if (item.key === s) {
                return item.value;
            }
        };
        return undefined;
    }

    public allElementsTemplateLoaded(onDone: any): boolean {
        for (let i: number = 0; i < this.elements.length; i++) {
            let item: any = this.elements[i];
            let templateName = item.template;
            let t: string;
            if (!(templateName === undefined)) {
                t = this.getTemplate(templateName);
                if (t === undefined) {
                    this.loadTemplateByName(templateName, onDone);
                    return false;
                }
            }
            templateName = item.editingtemplate
            if (!(templateName === undefined)) {
                t = this.getTemplate(templateName);
                if (t === undefined) {
                    this.loadTemplateByName(templateName, onDone);
                    return false;
                }
            }
            templateName = item.visual;
            if (!(templateName === undefined)) {
                t = this.getTemplate(templateName);
                if (t === undefined) {
                    this.loadTemplateByName(templateName, onDone);
                    return false;
                }
            }

        };
        return true;
    }


    public loadElementsTemplate(): void {
        let allReady: boolean = this.allElementsTemplateLoaded(() => this.loadElementsTemplate());
        if (allReady === false) return;
        this.allLoaded();
        Converters.ConvertAll();
        $(".switch-checkbox[data-state='ON']").prop("checked", true);
    }

    public allLoaded(): void {

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
}

class HomePage extends Model {


    public init(): void {
        super.init();
        this.loadProcesses();
    }

    public loadProcesses(): void {
        $.get(WebUI.rooturl + "switches").done(
            (data: any, status: any) => this.ProcessesLoaded(data, status)
        ).fail(
            (data: any, status: any) => this.loadFailed(data, this.loadProcesses)
        );
    }

    private ProcessesLoaded(data: any, status: any): void {
        this.updateTime(data);
        let w: Items_list = data;
        let list: string = "";
        for (let i: number = 0; i < w.items.length; i++) {
            this.elements.push(w.items[i]);
        };
        this.loadElementsTemplate();
    }

    public allLoaded() {
        let list: string = "";
        for (let i: number = 0; i < this.elements.length; i++) {
            let item: any = this.elements[i];

            if (item.visual) {
                let t: string = this.getTemplate(item.visual);
                if (t === undefined) {
                    list += "<li class='nav-item'>";
                    list += item.name;
                    list += "</li>";
                } else {
                    list += this.fillTemplate(t, item);
                }
            }
        }
        $(".process-list").html(list);

        super.allLoaded();

        $("img[data-state='ON']").removeClass("light-off");

        $(".switch-img").click((e: any) => this.switch_img_click(e))
        $(".switch-checkbox").change((e: any) => this.switch_checkbox_change(e));

    }

    public switch_img_click(event: any): void {

        let cb = $(".switch-checkbox", $(event.target).closest(".card"));
        if (cb.data("state") == "ON") {
            this.turnOff(cb.data("switch"));
        } else {
            this.turnOn(cb.data("switch"));
        }
    }

    public switch_checkbox_change(event: any): void {

        if (event.target.checked) {
            this.turnOn($(event.target).data("switch"))
        } else {
            this.turnOff($(event.target).data("switch"))
        }
    }


    public turnOn(i: number): boolean {
        if ($("#switch_" + i.toString()).data("state") == "ON") return;

        console.log("turn on");
        $.post(WebUI.rooturl + "switches" + "?index=" + i.toString() + "&state=on").done(
            function (data: any, status: any) {
                if (data.systime) { $("#systime").html(data.systime); };
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
        $.post(WebUI.rooturl + "switches" + "?index=" + i.toString() + "&state=off").done(
            function (data: any, status: any) {
                if (data.systime) { $("#systime").html(data.systime); };
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

}

class SetupPage extends Model {


    public init(): void {
        super.init();
        this.loadTriggers();
    }

    public loadTriggers(): void {
        let index: number = this.getUrlParameter("index");
        $.get(WebUI.rooturl + "setup?type=switch&index=" + index.toString()).done(
            (data: any, status: any) => this.TriggersLoaded(data, status)
        ).fail(
            (data: any, status: any) => this.loadFailed(data, this.loadTriggers)
        );
    }

    TriggersLoaded(data: any, status: any): any {
        this.updateTime(data);
        let w: Items_list = data;
        let list: string = "";
        for (let i: number = 0; i < w.items.length; i++) {
            this.elements.push(w.items[i]);
        };
        this.loadElementsTemplate();
    }

    public allLoaded() {
        let allReady: boolean = true;

        let t = this.getTemplate("newitem");
        if (t === undefined) {
            allReady = false;
            this.loadTemplateByName("newitem", () => this.allLoaded());
            return;
        }


        if (allReady === true) {
            let list: string = "";

            list += this.getTemplate("newitem");

            for (let i: number = 0; i < this.elements.length; i++) {
                let item: Trigger = this.elements[i];
                let t: string = this.getTemplate(item.template);
                list += "<div class='holder' id='trigger_" + item.uid + "' data-index='" + i.toString() + "' data-uid='" + item.uid + "'>" + this.fillTemplate(t, item) + "</div>";
            }

            $(".process-list").html(list);
            $(".btn-edit").click((e) => this.edit(e));
            $(".btn-add").click((e) => this.add(e));
            $('.btn-delete').click((e) => this.delete(e));
            $(".switch-checkbox[data-state='on']").prop("checked", true);
            Converters.ClockPicker();
        }

        $(".switch-checkbox[data-state='ON']").prop("checked", true);
        $("img[data-state='ON']").removeClass("light-off");
        Converters.ConvertAll();
    }

    public add(e: any): void {
        console.log("add");

        let item: Trigger = $(e.target).data("newitem");
        let t: string = this.getTemplate(item.editingtemplate);
        if (t === undefined) {
            let _e: any = e;
            this.loadTemplateByName(item.editingtemplate, () => this.add(_e));
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
        $('.btn-save').click((e) => this.save(e));

        $(".switch-checkbox[data-state='on']").prop("checked", true);
        Converters.ConvertAll();
        Converters.ClockPicker();
    }

    public edit(e: any): void {
        console.log("edit");
        let holder: JQuery = $(e.target).closest(".holder");
        holder.data("edit", true);
        let i: number = holder.data("index");
        let item: Trigger = this.elements[i];
        let t: string = this.getTemplate(item.editingtemplate);
        holder.html(this.fillTemplate(t, item));


        $('.btn-edit').attr("disabled", "disabled");
        $('.btn-save').click((e) => this.save(e));


        $(".switch-checkbox[data-state='on']").prop("checked", true);
        Converters.ConvertAll();
        Converters.ClockPicker();
    }

    public save(e: any): void {
        console.log("save");
        let holder: JQuery = $(e.target).closest(".holder");
        let form: JQuery = $("form", holder);
        let fields = $("[name]", form);
        let url: string = WebUI.rooturl + "setup?switch=" + this.getUrlParameter("index") + "&uid=" + holder.data("uid");
        for (let i: number = 0; i < fields.length; i++) {
            let v: string;
            let f = $(fields[i]);
            if (f.hasClass("converter-time")) v = TimeConverter.ConvertBack(fields[i]);
            else if (f.hasClass("converter-week-days")) v = WeekDaysConverter.ConvertBack(fields[i]);
            else v = TextConverter.ConvertBack(fields[i]);
            url += "&" + fields[i].getAttribute("name") + "=" + encodeURIComponent(v);
        }
        $.get(url).done(function (data: any, status: any) {
            location.reload();
        }
        ).fail(function (data: any, status: any) {
            if (data.systime) { $("#systime").html(data.systime); };
            alert(data);
        }
        );
    }

    public delete(e: any): void {
        console.log("delete");
        let _e = e;
        WebUI.ask("Вилучити тригер зі списку?", () => this.onDeleConfirmed(_e));
    }

    private onDeleConfirmed(e: any) {
        let holder: JQuery = $(e.target).closest(".holder");
        let form: JQuery = $("form", holder);
        //let fields = $("[name]", form);
        let url: string = WebUI.rooturl + "setup?switch=" + this.getUrlParameter("index") + "&delete=" + holder.data("uid");
        $.get(url).done(function (data: any, status: any) {
            location.reload();
        }
        ).fail(function (data: any, status: any) {
            if (data.systime) { $("#systime").html(data.systime); };
            alert(data);
        }
        );
    }

}

class WifiPage extends Model {

    public init(): void {
        super.init();
        this.loadStationList();
    }

    public loadStationList(): void {
        $.get(WebUI.rooturl + "wifi").done(
            (data: any, status: any) => this.StationListLoaded(data, status)
        ).fail(
            (data: any, status: any) => this.loadFailed(data, this.loadStationList)
        );
    }

    private StationListLoaded(data: any, status: any): void {
        this.updateTime(data);
        let w: WIFI_list = data;
        $(".localip").text(w.localip);
        $(".mac").text(w.mac);
        $(".dnsip").text(w.dnsip);
        $(".getway").text(w.getway);
        let list: string = "";
        for (let i: number = 0; i < w.ssid.length; i++) {
            let item: any = w.ssid[i];
            item.template = "wifi";
            if (item.encryption == "7") {
                item.encryption_name = "-";
            } else {
                item.encryption_name = "***";
            }
            this.elements.push(item);
        };

        this.loadElementsTemplate();
    }

    public allLoaded() {
        let list: string = "";
        for (let i: number = 0; i < this.elements.length; i++) {
            let item: any = this.elements[i];
            let t: string = this.getTemplate(item.template);
            if (t === undefined) {
                list += "<li class='nav-item'>";
                list += item.name;
                list += "</li>";
            } else {
                list += this.fillTemplate(t, item);
            }
        };
        $(".wifi-list").html(list);
        $(".wifi-item").on("click", (e) => this.wifi_item_click(e));
    }

    private wifi_item_click(event: any) {
        $('#ssid').val($(".name", event.currentTarget).text());
        $('#password').focus();
    }

}

class WIFI_list {
    public mac: string;
    public localip: string;
    public dnsip: string;
    public getway: string;
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

class Converters {

    public static ConvertAll() {
        TimeConverter.Convert();
        TextConverter.Convert();
        WeekDaysConverter.Convert();
    }

    public static ClockPicker() {
        let cp: any = $(".clockpicker");
        cp.clockpicker({
            placement: 'top',
            autoclose: true
        });
    }
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

    public static Convert(): void { }

    public static ConvertBack(elem: Element): string {
        let el = $(elem);
        if (el.attr("type") === "checkbox") {
            return el.prop("checked");
        }
        return el.val();
    }

}

class WeekDaysConverter {


    //static weekday: any = ["Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"]
    static weekday: string[] = ["Нд", "Пн", "Вт", "Ср", "Чт", "Пт", "Сб"]

    public static Convert(): void {

        let inputs = $(".converter-week-days")
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

                for (let day: number = 0; day < 7; day++) {
                    if ((i & (1 << day)) != 0) {
                        if (res.length > 0) res += ", ";
                        res += WeekDaysConverter.weekday[day];
                    }
                }

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
            let parts = v.split(",");
            let res: number = 0;
            for (let i: number = 0; i < parts.length; i++) {
                let n: number = WeekDaysConverter.weekday.indexOf(parts[i].trim());
                if (n >= 0) res += (1 << n);
            }
            return res.toString();
        }
        return el.val();
    }

}