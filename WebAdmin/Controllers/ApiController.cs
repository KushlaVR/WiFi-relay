using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Hosting;
using Microsoft.AspNetCore.Mvc;
using WebAdmin.Models;
using System.Runtime.InteropServices;

namespace WebAdmin.Controllers
{
    public class ApiController : Controller
    {

        public class Trigger
        {
            private static int _uid = 0;

            public Trigger()
            {
                _uid++;
                uid = _uid.ToString();
            }

            public string uid { get; set; }
            public string type { get; set; } = "abstract";
            public string name { get; set; }
            public string template { get; set; } = "trigger";
            public string editingtemplate { get; set; } = "edittrigger";


        }

        public class OnOffTrigger : Trigger
        {
            public OnOffTrigger()
            {
                type = "onoff";
                template = "onoff";
                editingtemplate = "onoffedit";
            }

            public string action { get; set; }
            public string days { get; set; } = "255"; //пн-пт
            public string time { get; set; } = "480";//08:00 = 8*60 + 00 = 480
        }

        public class PWMTrigger : Trigger
        {

            public PWMTrigger()
            {
                type = "pwm";
                template = "pwm";
                editingtemplate = "pwmedit";
            }

            public string days { get; set; } = "255"; //пн-пт
            public string onlength { get; set; } = "5";// 00:05
            public string offlength { get; set; } = "15";// 00:05




        }


        public class MQTTProcess
        {
            public string name { get; set; }
            public string visual { get; set; } = "switch";
            public string type { get; set; } = "switch";
            public string state { get; set; } = "OFF";
            public string index { get; set; } = "0";
        }

        private static List<MQTTProcess> items;
        private static Dictionary<MQTTProcess, List<Trigger>> triggers;
        private IHostingEnvironment _env;
        public ApiController(IHostingEnvironment env)
        {
            _env = env;
            if (items == null)
            {
                items = new List<MQTTProcess>();
                triggers = new Dictionary<MQTTProcess, List<Trigger>>();
                MQTTProcess item = new MQTTProcess() { name = "out1", index = "1" };
                items.Add(item);
                triggers.Add(item, new List<Trigger>());
                triggers[item].Add(new OnOffTrigger() { name = "Включати 8:00", action = "on", time = "480" });
                triggers[item].Add(new OnOffTrigger() { name = "Виключати 9:00", action = "off", time = "540" });

                items.Add(new MQTTProcess() { name = "out2", index = "2" });
                items.Add(new MQTTProcess() { name = "out3", index = "3" });

                item = new MQTTProcess() { name = "led", index = "4", state = "ON" };
                items.Add(item);
                triggers.Add(item, new List<Trigger>());
                triggers[item].Add(new PWMTrigger());

            }

        }

        public IActionResult Index()
        {
            var webRoot = _env.WebRootPath;
            var file = System.IO.Path.Combine(webRoot, "test.txt");
            System.IO.File.WriteAllText(file, "Hello World!");
            return View();
        }
        public IActionResult Get()
        {
            var webRoot = _env.WebRootPath + "\\content\\";

            if (System.IO.Directory.Exists(webRoot))
            {
                string ret = "";
                string var = null;
                String content = System.IO.File.ReadAllText(webRoot + Request.Query["name"] + ".txt");
                int i = 0;
                while (i < content.Length)
                {
                    char c = content[i];
                    if (var == null)
                    {
                        if (c == '[') { var = ""; } else { ret += c; }
                    }
                    else
                    {
                        if (c == ']')
                        {
                            ret += Request.Query[var];
                            var = null;
                        }
                        else { var += c; }
                    }
                    i++;
                };
                return Content(ret);
            }
            return NotFound();
        }

        public IActionResult wifi()
        {
            return Content("{\"ssid\":[{\"name\":\"KOLUMBUS\",\"encryption\":\"4\",\"rssi\":\"-55\"},{\"name\":\"netis_5D1566\",\"encryption\":\"8\",\"rssi\":\"-94\"},{\"name\":\"Ukrtelecom_105\",\"encryption\":\"2\",\"rssi\":\"-64\"},{\"name\":\"TODD0205\",\"encryption\":\"4\",\"rssi\":\"-71\"},{\"name\":\"codename47\",\"encryption\":\"4\",\"rssi\":\"-88\"},{\"name\":\"Prosvity19_98\",\"encryption\":\"4\",\"rssi\":\"-52\"},{\"name\":\"KushlaVR\",\"encryption\":\"4\",\"rssi\":\"-55\"},{\"name\":\"anastasia\",\"encryption\":\"8\",\"rssi\":\"-61\"},{\"name\":\"TP-LINK_44C0\",\"encryption\":\"4\",\"rssi\":\"-76\"},{\"name\":\"TP-LINK_zyzja\",\"encryption\":\"7\",\"rssi\":\"-76\"},{\"name\":\"lyudmyla\",\"encryption\":\"4\",\"rssi\":\"-78\"},{\"name\":\"Volia_93\",\"encryption\":\"4\",\"rssi\":\"-74\"},{\"name\":\"TerNet99\",\"encryption\":\"8\",\"rssi\":\"-72\"},{\"name\":\"Oksana\",\"encryption\":\"8\",\"rssi\":\"-83\"},{\"name\":\"Bandera\",\"encryption\":\"4\",\"rssi\":\"-89\"},{\"name\":\"Svitlana_Wi-Fi\",\"encryption\":\"8\",\"rssi\":\"-83\"}]}", new Microsoft.Net.Http.Headers.MediaTypeHeaderValue("application/json"));
        }


        public IActionResult wifisave(string n, string p)
        {
            return new JsonResult(new { result = "ok" });
        }

        [HttpGet()]
        public IActionResult switches()
        {
            return new JsonResult(new { items = items });
        }

        [HttpPost()]
        public IActionResult switches(int? index, string state)
        {
            if (index.HasValue && !string.IsNullOrEmpty(state))
            {
                if (index.Value <= items.Count)
                {
                    items[index.Value - 1].state = state.ToUpper();
                    return new JsonResult(new { status = "OK" });
                }
            }
            return NotFound();
        }

        [HttpGet()]
        public IActionResult template(string name)
        {
            var webRoot = _env.WebRootPath;
            var file = System.IO.Path.Combine(webRoot, "content/_" + name + ".html");
            //System.IO.File.WriteAllText(file, "Hello World!");
            return Content(System.IO.File.ReadAllText(file));
        }


        [HttpGet()]
        public IActionResult setup()
        {

            string type = Request.Query["type"];
            string sindex = Request.Query["index"];
            if (type == "switch")
            {
                List<Trigger> list = new List<Trigger>();
                MQTTProcess item = items[int.Parse(sindex) - 1];
                return Json(new { items = triggers[item].ToArray() });
            }
            else if (type == "onoff")
            {
                MQTTProcess item = items[int.Parse(Request.Query["switch"]) - 1];
                string uid = Request.Query["uid"];

                if (uid == "0")
                {

                    OnOffTrigger tr = new OnOffTrigger();
                    tr.time = Request.Query["time"];
                    tr.days = Request.Query["days"];
                    tr.name = Request.Query["name"];
                    if (Request.Query["action"] == "true")
                    {
                        tr.action = "on";
                    }
                    else
                    {
                        tr.action = "off";
                    }
                    triggers[item].Add(tr);
                    return new JsonResult(new { status = "OK" });
                }
                else
                {
                    foreach (Trigger t in triggers[item])
                    {
                        if (t.uid == uid)
                        {
                            if (t.type == "onoff")
                            {
                                OnOffTrigger tr = t as OnOffTrigger;
                                tr.time = Request.Query["time"];
                                tr.days = Request.Query["days"];
                                tr.name = Request.Query["name"];
                                if (Request.Query["action"] == "true")
                                {
                                    tr.action = "on";
                                }
                                else
                                {
                                    tr.action = "off";
                                }
                                return new JsonResult(new { status = "OK" });
                            }
                        }
                    }
                }

            }
            else if (type == "pwm")
            {
                MQTTProcess item = items[int.Parse(Request.Query["switch"]) - 1];
                string uid = Request.Query["uid"];

                if (uid == "0")
                {
                    PWMTrigger tr = new PWMTrigger();
                    tr.onlength = Request.Query["onlength"];
                    tr.offlength = Request.Query["offlength"];
                    tr.days = Request.Query["days"];
                    tr.name = Request.Query["name"];
                    triggers[item].Add(tr);
                    return new JsonResult(new { status = "OK" });
                }
                else
                {
                    foreach (Trigger t in triggers[item])
                    {
                        if (t.uid == uid)
                        {
                            if (t.type == "pwm")
                            {
                                PWMTrigger tr = t as PWMTrigger;
                                tr.onlength = Request.Query["onlength"];
                                tr.offlength = Request.Query["offlength"];
                                tr.days = Request.Query["days"];
                                tr.name = Request.Query["name"];
                                return new JsonResult(new { status = "OK" });
                            }
                        }
                    }
                }
            }
            return NotFound();
        }

    }
}
