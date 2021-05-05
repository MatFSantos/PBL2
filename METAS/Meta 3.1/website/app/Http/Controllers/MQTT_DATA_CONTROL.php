<?php

namespace App\Http\Controllers;
use PhpMqtt\Client\Facades\MQTT;
use Illuminate\Http\Request;

class MQTT_DATA_CONTROL extends Controller
{


    public function home(){
        $state = "NaN";
        $verify = "NULL";
        return view('home', compact('state','verify'));
    }
    public function mqtt_function($state){
        $mqtt = MQTT::connection();
        $verify = "NULL";
        if($state == "att"){
            $mqtt->publish('ON_OFF', '', 0);
            return view('home', compact('state','verify'));
        }
            return view('home', compact('state','verify'));             
    }
}
