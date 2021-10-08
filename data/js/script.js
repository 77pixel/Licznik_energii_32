window.onload = () => {
    'use strict';
  
    if ('serviceWorker' in navigator) {
      navigator.serviceWorker
               .register('./sw.js');
    }
  }

function aktdane(param)
{
    $.post("/dane", param).done(function(data, status) 
    {
        $('#suma_ampery').html(Number(data.f1_ampery) + Number(data.f2_ampery)+ Number(data.f3_ampery));
        $('#suma_waty').html(Number(data.f1_waty) + Number(data.f2_waty)+ Number(data.f3_waty));
        $('#suma_kilos').html(Number(data.f1_kilos) + Number(data.f2_kilos)+ Number(data.f3_kilos));
        $.each(data, function(key,val){$('#'+key).html(val);});
        var suma=[];
        data.f1_dane.forEach(function (item, index){suma.push(Number(data.f1_dane[index]) + Number(data.f2_dane[index])+ Number(data.f3_dane[index]))});
        

        // wykres.series[0].update({
        //     pointStart: newSeries[0].pointStart,
        //     data: newSeries[0].data
        // }, true); //true / false to redraw

        var serie =[{"name": "Faza 1", "data":data.f1_dane},
                    {"name": "Faza 2", "data":data.f2_dane},
                    {"name": "Faza 3", "data":data.f3_dane},
                    {"name": "Suma", "type": "spline", "data":suma}];
        
        Highcharts.chart('container', 
        {
            title: {
                text: 'Zużycie energii'
            },
            yAxis: {
                opposite: true,
                title: {
                    text: 'Pobór w Watach'
                }
            },
            xAxis: {
                reversed: true
            },
            legend: {
                layout: 'vertical',
                align: 'right',
                verticalAlign: 'middle'
            },
            series: serie,
            plotOptions: {
                series: {
                    animation: {duration:0}
                    }
                },
            responsive: {
                rules: [{
                    condition: {
                        maxWidth: 500
                    },
                    chartOptions: {
                        legend: {
                            layout: 'horizontal',
                            align: 'center',
                            verticalAlign: 'bottom'
                        }
                    }
                }]
            }
        });
    });
}

$(function() 
{
    aktdane('');
    setInterval(function() {aktdane('');}, 10000);
});

