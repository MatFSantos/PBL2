<?php

namespace App\Models;

use Illuminate\Database\Eloquent\Factories\HasFactory;
use Illuminate\Database\Eloquent\Model;
use PhpMqtt\Client\Facades\MQTT;

class MQTT_MODEL extends Model
{
    use HasFactory;

    

    public function run(){
        return $mqtt->subscribe('STATE', function (string $topic, string $message) {
            echo sprintf('Received QoS level 0 message on topic [%s]: %s', $topic, $message);    
            
        }, 0);


    }
}
