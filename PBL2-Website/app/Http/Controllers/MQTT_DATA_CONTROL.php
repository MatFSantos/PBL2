<?php

namespace App\Http\Controllers;
use PhpMqtt\Client\Facades\MQTT;
use Illuminate\Http\Request;
use AWS;
use Config;

class MQTT_DATA_CONTROL extends Controller
{
    public $valorEnergy;

    public function home(){
        $state = "NaN";
        $verify = "NaN";
        return view('home', compact('state','verify'));
    }
    public function changeState(){
        $mqtt = MQTT::connection();
        $mqtt->publish('ON_OFF', '', 0);
        $verify = $this->atualizer($mqtt);
        $status = explode(':', $verify);
        $status = explode('"', $status[1]);  
        $mqtt->disconnect();
        $stateSuccess['success'] = true;
        return response()->json(['verify'=>$verify, 'state'=> $status[1]]);

    }
    public function scheduler(Request $request){
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

    public function atualizerDB(){
        $timeActive = "";
        $verify = "";
        $state = "";
        $mqtt = MQTT::connection();

        $mqtt->subscribe('TEMPO_ATIVO', function (string $topic , string $message, bool  $retained) use ($mqtt){
            $this->timeActive = $message;
            $mqtt->interrupt();
        }, 0);
        $mqtt->loop(true);

        $timeActive = explode(':', $timeActive);
        $timeActive = explode('"', $timeActive[1]);

        $mqtt->subscribe('VERIFICAR', function (string $topic , string $message, bool  $retained) use ($mqtt){
            $this->verify = $message;
            $mqtt->interrupt();
        }, 0);
        $mqtt->loop(true);

        $verify = explode(':', $verify);
        $verify = explode('"', $verify[1]);

        $state = atualizer($mqtt);

        $mqtt->disconnect();


    }

}
