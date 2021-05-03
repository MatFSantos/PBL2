<?php

namespace Database\Factories;

use App\Models\Log;
use Illuminate\Database\Eloquent\Factories\Factory;

class logfactoryFactory extends Factory
{
    /**
     * The name of the factory's corresponding model.
     *
     * @var string
     */
    protected $model = Log::class;

    /**
     * Define the model's default state.
     *
     * @return array
     */
    public function definition()
    {
        $energia = this.faker->randomFloat($nbMaxDecimals = 6, $min = 0.2, $max = 0.8)
        return [
            'data' => "24/04/2021";
            'energia' => $energia;
            'custo'=> $energia*0.41544427;
        ];
    }
}
