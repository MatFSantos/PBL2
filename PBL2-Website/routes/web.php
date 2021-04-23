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
Route::get('', 'App\Http\Controllers\MQTT_DATA_CONTROL@home')->name('home');
Route::get('att', 'App\Http\Controllers\MQTT_DATA_CONTROL@change_state')->name('att');
Route::get('schedule', 'App\Http\Controllers\MQTT_DATA_CONTROL@scheduler')->name('schedule');
//[MQTT_DATA_CONTROL::class, 'scheduler']