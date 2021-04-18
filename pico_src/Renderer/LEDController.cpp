#include "LEDController.hpp"

#include "pico/time.h"

#include "apa102.pio.h"
#include "RTTMeasure.hpp"

PIO LEDController::pio{};
uint LEDController::sm = 0;
uint8_t LEDController::pixel_buffer[2][N_BUFFER_SIZE];
critical_section_t LEDController::crit = {};
uint8_t LEDController::render_buffer_idx = 0;

LEDController::LEDController()
{
    critical_section_init(&crit);

    multicore_launch_core1(&LEDController::core1_write_pixels);   
}

LEDController::~LEDController()
{
}

LEDController& LEDController::getInstance() {
    static LEDController instance;
    return instance;
}

void LEDController::swapBuffers() {
    render_buffer_idx = (render_buffer_idx + 1) % 2;
}
uint8_t* LEDController::getRenderBuffer() {
    return pixel_buffer[render_buffer_idx];
}
uint8_t* LEDController::getInputBuffer() {
    return pixel_buffer[(render_buffer_idx + 1) % 2];
}

void LEDController::put_start_frame(PIO pio, uint sm) {
    pio_sm_put_blocking(pio, sm, 0u);
}

void LEDController::put_end_frame(PIO pio, uint sm) {
    pio_sm_put_blocking(pio, sm, ~0u);
}

void LEDController::put_rgb888(PIO pio, uint sm, uint8_t r, uint8_t g, uint8_t b) {
    pio_sm_put_blocking(pio, sm,
                        0x7 << 29 |                   // magic
                        (BRIGHTNESS & 0x1f) << 24 |   // global brightness parameter
                        (uint32_t) b << 16 |
                        (uint32_t) g << 8 |
                        (uint32_t) r << 0
    );
}

void LEDController::core1_write_pixels(){
    pio = pio0;
    sm = 0;
    uint offset = pio_add_program(pio, &apa102_mini_program);
    apa102_mini_program_init(pio, sm, offset, SERIAL_FREQ, PIN_CLK, PIN_DIN);
    
    RTTMeasure& rttMeasure = RTTMeasure::getInstance();
    LEDController& ledController = getInstance();
    
    uint32_t column = 0;
    uint32_t last_column = 0xFFFFFF;
    bool last_cycle_rotation_detected = false;
    while (true) {   
        if (rttMeasure.rotationDetected()){
            last_cycle_rotation_detected = true;
            const absolute_time_t frame_start = get_absolute_time();   
            const uint32_t column = rttMeasure.getCurrentColumn(N_HORIZONTAL_RESOLUTION);

            if (last_column != column){
                if ((column - last_column > 1) && (last_column < column)){
                  //printf("Skipped column %u %u\n", column, last_column);
                } 
                last_column = column;

                critical_section_enter_blocking(&crit);
                uint8_t* render_buff = ledController.getRenderBuffer();

                ledController.put_start_frame(pio, sm);
                const uint8_t* pixel_buffer_column = render_buff + column*N_VERTICAL_RESOLUTION*N_CHANNELS_PER_PIXEL;
                for (int i = 0; i < N_VERTICAL_RESOLUTION*N_CHANNELS_PER_PIXEL; i+=N_CHANNELS_PER_PIXEL) {
                    ledController.put_rgb888(pio, sm,
                            pixel_buffer_column[i],
                            pixel_buffer_column[i+1],
                            pixel_buffer_column[i+2]
                    );
                }
                // Double sided globe
                const uint32_t opposite_column = (column + N_HORIZONTAL_RESOLUTION/2) % N_HORIZONTAL_RESOLUTION;
                const uint8_t* pixel_buffer_opposite_column = render_buff + opposite_column*N_VERTICAL_RESOLUTION*N_CHANNELS_PER_PIXEL;
                for (int i = N_VERTICAL_RESOLUTION*N_CHANNELS_PER_PIXEL-1; i >=0; i-=N_CHANNELS_PER_PIXEL) {
                    ledController.put_rgb888(pio, sm,
                            pixel_buffer_opposite_column[i-2],
                            pixel_buffer_opposite_column[i-1],
                            pixel_buffer_opposite_column[i-0]
                    );
                }
                ledController.put_end_frame(pio, sm);
                critical_section_exit(&crit);
            }
        }else if(last_cycle_rotation_detected) {
            printf("No rotation detected.\n");
            last_cycle_rotation_detected = false;
        }
    }
}


critical_section_t* LEDController::getCriticalSection() {
    return &crit;
}