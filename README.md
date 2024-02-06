# ESP32CAM
Date: 03/02/2024
  ESP32cam takes the photo and uploads it to the model via the API to recognise the image and finally receive the response.
  
Date: 04/02/2024
  There are currently three issues:
  1) The image quality can only be set to VGA or lower ("config.frame_size = FRAMESIZE_VGA"). Otherwise, it results in an incomplete request body, manifested by the omission of the "token" after "image."
  2) This is currently the most significant issue: when taking photos of different objects by inputting "TAKE" twice, the second photo remains the same as the first one. A new photo only appears when taking the third shot. Similarly, the photo taken on the fourth attempt still shows the third shot.
  3) The server response is unstable, and failures are frequent.
