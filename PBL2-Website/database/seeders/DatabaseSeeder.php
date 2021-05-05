<?php

namespace Database\Seeders;

use Illuminate\Database\Seeder;
use App\Models\Log;
use App\Models\Statu;
use Illuminate\Database\Factories\LogFactory;
use Illuminate\Database\Factories\StatuFactory;

class DatabaseSeeder extends Seeder
{
    /**
     * Seed the application's database.
     *
     * @return void
     */
    public function run()
    {
         Log::factory(10)->create();
         //Statu::factory()->create();
    }
}
