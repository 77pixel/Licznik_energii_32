#include "wifisetup.h"

//Konfiguracja konfig = Konfiguracja("config");

int WIFIsetup()
{
  String txt = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'><meta charset='UTF-8'></head><bodystyle='font-family: Tahoma, sans-serif;>";

  if(server.method() == HTTP_POST)
  {
    if (server.hasArg("naz"))
    { 
        String n = server.arg("nazwau");
        //konfig.zapisz(0, n);
        txt +="<h1> Zapisano nazwę. Zrestartuj urządzenie...</h1>";
    }
    else if(server.hasArg("res"))
    {
        ESP.restart();
    }
    else
    { 
        String ssid = server.arg("ssid");
        //konfig.zapisz(1, ssid);
        
        String pass = server.arg("pass");
        //konfig.zapisz(2, pass);

        txt +="<h1> Zapisano sieć WiFi. Zrestartuj urządzenie...</h1>";
    }
  }
  else
  {
    if (WiFi.status() == WL_CONNECTED)
    {
        txt += "<h3>Połączono z <b>" + WiFi.SSID() + " (" + WiFi.localIP().toString() + ")</b></h3>";
    }

    int n = WiFi.scanNetworks();
    if (n == 0) 
    {
        txt += "<h1>BRAK SIECI</h1>";
    } 
    else 
    {
        txt +="<div style=''><h3>Lista znalezionych sieci:</h3>";
        txt +="<table id='tab' style=';'><thead><tr><th></th><th>Nazwa</th><th>Moc</th><th>Kanał</th><th>Zab.</th></tr></thead><tbody>";
        
        for (int i = 0; i < n; ++i) 
        {
            String enc = "??"; //(WiFi.encryptionType(i) == ENC_TY) ? "Brak" : "Tak";
            
            txt +="<tr style='cursor: pointer;'>";

            txt +="<td>"  + String(i+1) + "</td>"
                + "<td>" + WiFi.SSID(i) + "</td>"
                + "<td>" + WiFi.RSSI(i) + " dB</td>"
                + "<td stytle='text-align: right;'>" + WiFi.channel(i) + "</td>"
                + "<td>" + enc + "</td>"
                + "</tr>";
        }

        txt +="</tbody></table></div>";
    }

    String nazwa = "test";

    txt += "<div id='con' style='display:none'><form method='post'>Nazwa: <input id='ssid' name='ssid' readonly><br>Hasło: <input id='pass' name='pass' type='password'><input type='submit' value='Zapisz'></form></div>";
    txt += "<div><form method='post'><input value='1' id='naz' name='naz' hidden>Nazwa urządzenia: <input id='nazwau' name='nazwau' value='"+nazwa+"'><input type='submit' value='Zmień'></form></div>";
    txt += "<div><form method='post'><input id='res' name='res' hidden><input type='submit' value='Reset'></form></div>";
    txt += "<script>";
    txt += "document.getElementById('tab').addEventListener('click', m);";
    txt += "function m(e){document.getElementById('ssid').value=e.target.parentNode.cells[1].innerText;document.getElementById('con').style.display='block'}";
    txt += "</script>"; 
  }

  txt +="</body></html>";
  server.send(200, "text/html", txt);
  return 1;
}