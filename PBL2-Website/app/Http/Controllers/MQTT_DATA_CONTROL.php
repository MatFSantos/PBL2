<?php

namespace App\Http\Controllers;
use PhpMqtt\Client\Facades\MQTT;
use Illuminate\Http\Request;
use App\Models\Log;
use App\Models\Statu;
use AWS;
use Config;

class MQTT_DATA_CONTROL extends Controller
{

    public function home(){
        $state = "NaN";
        $verify = "NaN";
        $logs = Log::all();
        $arrLogs = [];
        //dd($logs->first());
        if($logs->first() != null){
            $arrLogs = $this->sumEqualDates();
        }
        return view('home', compact('state','verify', 'arrLogs'));
    }
    public function changeState(){
        $mqtt = MQTT::connection();
        $mqtt->publish('ON_OFF', '', 0);  
        $mqtt->disconnect();
        $stateSuccess['success'] = true;
        

        return $this->atualizerWeb();

    }
    public function scheduler(Request $request){
        $schedule = $request->data['cont2'];
        $mqtt = MQTT::connection();
        $mqtt->publish('TEMPORIZADOR', $schedule, 0);
        $mqtt->disconnect();
        $stateSuccess['success'] = true;
         
        return $this->atualizerWeb();
    }


    public function atualizerWeb(){
        $state = Statu::all()->first()->estado;
        $arrLogs = $this->sumEqualDates();
        $stateSuccess['success'] = true;
        return response()->json(['state'=>$state, 'logs'=>$arrLogs]); 
    }

    public function sumEqualDates(){
        $logs = Log::all();
        $data = $logs->first()->data;
        $arrLogs = [];
        $sumEnergy = 0;
        $sumCust = 0;
        foreach ($logs as $log){                      
            if($data == $log->data){
                $sumEnergy += $log->energia;
                $sumCust += $log->custo;
            }
            else{
                $logAux = new Log;
                $logAux->data = $data;
                $logAux->energia = $sumEnergy;
                $logAux->custo = $sumCust;
                $arrLogs[] = $logAux;
                $data = $log->data;
                $sumEnergy = 0;
                $sumCust = 0;
                $sumEnergy = $log->energia;            
                $sumCust = $log->custo;       
            }
            
        }
        $logAux = new Log;             
        $logAux->data = $data;
        $logAux->energia = $sumEnergy;
        $logAux->custo = $sumCust;
        $arrLogs[] = $logAux;
        return $arrLogs;
    }

}
