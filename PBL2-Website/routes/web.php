<?php

use Illuminate\Support\Facades\Route;

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
Route::get('home', 'App\Http\Controllers\MQTT_DATA_CONTROL@home')->name('home');
Route::get('home/{state}', 'App\Http\Controllers\MQTT_DATA_CONTROL@mqtt_function')->name('home.mqtt');