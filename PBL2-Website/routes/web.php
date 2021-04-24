<?php

use Illuminate\Support\Facades\Route;
use App\Http\Controllers\MQTT_DATA_CONTROL;
/*
|--------------------------------------------------------------------------
| Web Routes
|--------------------------------------------------------------------------
|
| Here is where you can register web routes for your application. These
| routes are loaded by the RouteServiceProvider within a group which
| contains the "web" middleware group. Now create something great!
|
*/
Route::get('', [MQTT_DATA_CONTROL::class, 'home'])->name('home');
Route::get('att', [MQTT_DATA_CONTROL::class, 'changeState'])->name('att');
Route::get('schedule', [MQTT_DATA_CONTROL::class, 'scheduler'])->name('schedule');
Route::get('atualizer', [MQTT_DATA_CONTROL::class, 'atualizer'])->name('atualizer');