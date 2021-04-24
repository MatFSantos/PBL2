<?php

namespace App\Http\Controllers;
use PhpMqtt\Client\Facades\MQTT;
use Illuminate\Http\Request;
use AWS;
use Config;

class MQTT_DATA_CONTROL extends Controller
{
    
    public function home(){
        $state = "NaN";
        $verify = "NaN";
        return view('home', compact('state','verify'));
    }
    public function changeState(){
        $verify = "opa, funcionou";
        $state = "tambem funcionando";
        $mqtt = MQTT::connection();
        $mqtt->publish('ON_OFF', '', 0);
        $verify = $this->atualizer($mqtt);
        $mqtt->disconnect();
        $stateSuccess['success'] = true;
        return response()->json(['verify'=>$verify, 'state'=> $state]);

    }
    public function scheduler(Request $request){
        $verify = "funcionando";
        $state = "tambem funcionando";
        $schedule = $request->data['cont2'];
        $mqtt = MQTT::connection();
        $mqtt->publish('TEMPORIZADOR', $schedule, 0);
        $mqtt->disconnect();
        $stateSuccess['success'] = true;
        return response()->json(['verify'=>$verify, 'state'=>$state]); 
    }

    public function atualizer($mqtt){
        $verify = "";
        $mqtt->subscribe('ESTADO', function (string $topic , string $message, bool  $retained) use ($mqtt){
            $this->verify = $message;
            $mqtt->interrupt();
        }, 0);
        $mqtt->loop(true);

        return $this->verify;

    }

}
