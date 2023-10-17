function send_config() {
    var delay_conf = document.getElementById("delay_config").value; 
    var wbh_ip = document.getElementById('wbh_ip').value; 
    var colour = document.getElementById('colour').value; 
    var run_value = document.getElementById('run').value; 
    var run_value = document.getElementById('run').value;
    window.open('http://'+ LocalIP +'config/delay=' + delay_conf + '/wbh_ip=' + wbh_ip + '/colour=' + colour + '/run=' + run_value + '/config_end','_self');
}