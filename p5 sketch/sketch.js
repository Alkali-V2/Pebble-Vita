// for reference:
// https://static.super-shop.com/433007-nooka-watch-zub-20-zot-wt-white.jpg

let circleRadius,
  circleGap,
  minuteBarX,
  minuteBarY,
  minuteBarWidth,
  minuteBarHeight,
  minuteBoxWidth;

function setup() {
  createCanvas(windowWidth, windowHeight);
  circleRadius = 20;
  circleGap = 4 * circleRadius;
  minuteBarX = -circleGap * 1.5 - 20;
  minuteBarY = -circleGap * 1.5 + 3 * circleGap - 18;
  minuteBarWidth = 340;
  minuteBarHeight = circleRadius * 2;

  minuteBoxWidth = (minuteBarWidth - 59) / 60;
  // calculate the width of each minute box

  setInterval(draw, 60000);
  // update the clock every minute
}

function draw() {
  // get the time
  let now = new Date();
  let hours = now.getHours() % 12;
  let minutes = now.getMinutes();
  let seconds = now.getSeconds();

  background(255);
  translate(width / 2, height / 2);

  
  
  // hours
  fill(150); // Dark gray
  noStroke();

  for (let row = 0; row < 3; row++) {
    let startX = -circleGap * 1.5;
    let startY = -circleGap * 1.5 + row * circleGap;
    for (let col = 0; col < 4; col++) {
      let hour = row * 4 + col + 1;
      if (hour <= hours) {
        fill(150); // dark gray for current hour
      } else {
        fill(220); // light gray for remaining hours
      }
      ellipse(startX, startY, circleRadius * 2, circleRadius * 2);
      startX += circleGap;
    }
  }
  
  

  // minutes
  fill(255); // white to draw lines between minutes
  rect(minuteBarX, minuteBarY, minuteBarWidth, minuteBarHeight);
  // fill boxes for current minutes
  fill(150); // dark gray
  for (let i = 0; i < 60; i++) {
    if (i < minutes) {
      rect(
        minuteBarX + i * minuteBoxWidth,
        minuteBarY,
        minuteBoxWidth - 1,
        minuteBarHeight
      );
    } else {
      fill(220); // light gray for the remaining minutes
      rect(
        minuteBarX + i * minuteBoxWidth,
        minuteBarY,
        minuteBoxWidth - 1,
        minuteBarHeight
      );
    }
  }
  
  
  //seconds
  fill(0)
  text(seconds, 0, 200);
  textAlign(CENTER);
  textFont('IBM Plex Mono');
  noFill();
  stroke(150);
  strokeWeight(1);
  circle(0, 196, 40);
}

function windowResized() {
  resizeCanvas(windowWidth, windowWidth);
  circleGap = 4 * circleRadius;
  minuteBarWidth = 340;
  minuteBarHeight = circleRadius * 2 + 4;
  minuteBoxWidth = (minuteBarWidth - 59) / 60; // calculate the width of minute
  minuteBarX = -circleGap * 1.5 - 20;
  minuteBarY = -circleGap * 1.5 + 3 * circleGap - 20;
}
