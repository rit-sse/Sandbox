#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>
#include "calibration.hpp"
#include <cstdio>

static float give = 20; // The amount of give before we detect a wall
static int variance = 4; // How big of a square to grab 4 is an 8x8

DepthBox::DepthBox() {
  top_wall = 0;
  bottom_wall = 0;
  left_wall = 0;
  right_wall = 0;
}

bool DepthBox::calibrated() {
  return top_wall && bottom_wall && left_wall && right_wall;
}

void DepthBox::getCenterValue(libfreenect2::Frame *depth) {
  float pre_avg = 0;
  float average = 0;
  int counter = 0;
  int missed = 0;
  center_of_box = (((depth->width * depth->height) / 2) + (depth->width / 2));

  for(size_t x = (center_of_box - variance); x < (center_of_box + variance); x++) {
    for(size_t y = (x - (depth->width * variance)); y < (x + (depth->width * variance)); y+= depth->width) {
      counter++;
      if(frame_data[y] != 0) {
        pre_avg += frame_data[y];
      } else {
        missed ++;
      }
    }
  }

  average = (pre_avg / (counter - missed)); // Average Center Value

  center_value = average - give;
}

void DepthBox::calibrate(libfreenect2::Frame *depth) {
  frame_data = (float*)depth->data;

  getCenterValue(depth); // Sets center_value and center_of_box

  // Find left wall
  for (size_t i = center_of_box; i > (center_of_box - (depth->width / 2)); i--) {
    if(frame_data[i] != 0 && frame_data[i] < center_value) {
      left_wall = (depth->width / 2) + 1 - (center_of_box - i); // Always add one because its square wall_height = frame_data[i];
      wall_height = frame_data[i];
      break;
    }
  }

  // Find right wall
  for (size_t i = center_of_box; i < ((center_of_box + (depth->width / 2)) - 1); i++) { // we are at middle right so subtract 1
    if(frame_data[i] != 0 && frame_data[i] < center_value) {
      right_wall = (depth->width / 2) + 1 + (i - center_of_box); // Always add one because its square
      if (frame_data[i] > wall_height) wall_height = frame_data[i];
      break;
    }
  }

  // Find top wall
  // Top is the bottom but that makes me confused so this is the top wall
  for (size_t i = center_of_box; i > (center_of_box - (depth->width * (depth->height / 2))); i-= depth->width) {
    if(frame_data[i] != 0 && frame_data[i] < center_value) {
      top_wall = (depth->height / 2) - ((center_of_box - i) / depth->width);
      if (frame_data[i] > wall_height) wall_height = frame_data[i];
      break;
    }
  }

  // Find bottom wall
  for (size_t i = center_of_box; i < (center_of_box + (depth->width * ((depth->height / 2) - 1))); i+= depth->width) {
    if(frame_data[i] != 0 && frame_data[i] < center_value) {
      bottom_wall = (depth->height / 2) + 1 + ((i - center_of_box) / depth->width); // Magical + 1
      if (frame_data[i] > wall_height) wall_height = frame_data[i];
      break;
    }
  }
  printf("L: %zu R: %zu T: %zu B: %zu\n", left_wall, right_wall, top_wall, bottom_wall);
}

bool DepthBox::showCalibrationSquare(libfreenect2::Frame *depth) {
  calibrate(depth);
  for (size_t i = ((top_wall + 1) * depth->width); i < (bottom_wall * depth->width); i+= depth->width) {
    for (size_t j = left_wall + 1; j < right_wall; j++) {
      frame_data[i + j] = 100;
    }
  }

  return true;
}

RedSquare::RedSquare(size_t l, size_t r, size_t t, size_t b) {
  leftWall = l;
  rightWall = r;
  topWall = t;
  bottomWall = b;
}

void RedSquare::draw() {
  for (size_t i = ((topPos + 1) * width); i < (bottomPos * width); i+= width) {
    for (size_t j = leftPos + 1; j < rightPos; j++) {
      // do somthing
    }
  }
}

void RedSquare::drawSquare() {
  size_t red_size = 32;

  leftPos = (width / 2) - red_size;
  rightPos = (width / 2) + (red_size - 1);
  topPos = (height / 2) - red_size;
  bottomPos = (height / 2) + (red_size - 1);

  draw();
}

bool RedSquare::moveLeft() {
  if (leftPos == 0) return false;
  leftPos--;
  draw();
  return true;
}

bool RedSquare::moveRight() {
  if (rightPos == width) return false;
  rightPos++;
  draw();
  return true;
}

bool RedSquare::moveUp() {
  if (topPos == 0) return false;
  topPos--;
  draw();
  return true;
}

bool RedSquare::moveDown() {
  if (bottomPos == height) return false;
  bottomPos++;
  draw();
  return true;
}
