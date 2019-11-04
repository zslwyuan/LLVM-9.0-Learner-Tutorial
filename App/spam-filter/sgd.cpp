#define NUM_FEATURES 800

// void computeGradient(   int grad[NUM_FEATURES],    int    feature[NUM_FEATURES],    int scale)
// {

// }

// // Update the parameter vector
// void updateParameter(   int param[NUM_FEATURES],   int grad[NUM_FEATURES],    int scale)
// {
//   for (int i = 0; i < NUM_FEATURES ; i++)
//   {
//       param[i] += scale * grad[i];
//   }
// }


void compute(int theta_local[NUM_FEATURES],              int training_instance[NUM_FEATURES],             int gradient[NUM_FEATURES],             int prob,             int step            )
{
  
  // compute gradient
  // computeGradient(gradient, training_instance, (prob));
  // update the param vector
  // updateParameter(theta_local, gradient, step);

  for (int i = 0; i < NUM_FEATURES ; i++)
  {
      gradient[i] = (prob * training_instance[i]);
  }
  for (int i = 0; i < NUM_FEATURES ; i++)
  {
      theta_local[i] += step * gradient[i];
  }
}

