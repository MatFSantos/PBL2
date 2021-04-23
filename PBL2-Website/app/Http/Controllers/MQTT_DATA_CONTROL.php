<?php

namespace App\Http\Controllers;
use PhpMqtt\Client\Facades\MQTT;
use Illuminate\Http\Request;

class MQTT_DATA_CONTROL extends Controller
{
    
    public function home(){
        $state = "NaN";
        $verify = "NaN";
        return view('home', compact('state','verify'));
    }
    public function change_state(){
        $verify = "opa, funcionou";
        $state = "tambem funcionando";
        $mqtt = MQTT::connection();
        $mqtt->publish('ON_OFF', '', 0);
        $stateSuccess['success'] = true;
        return response()->json(['verify'=>$verify, 'state'=> $state]);

    }
    public function scheduler(Request $request){
        $verify = "funcionando";
        $state = "tambem funcionando";
        //$menssage = " "schedule" ";
        $schedule = $request->data['cont2'];
        $mqtt = MQTT::connection();
        $mqtt->publish('TEMPORIZADOR', $schedule, 0);
        $stateSuccess['success'] = true;
        return response()->json(['verify'=>$verify, 'state'=>$state]); 
    }

}
