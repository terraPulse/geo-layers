/*-----------------------------------------------------------
|
|  Routine Name: contable - Compute contigency table and statistics
|
|       Purpose: Main function for the contable program
|
|         Input: 
|
|        Output: 
|
|       Returns: TRUE (1) on success, FALSE (0) on failure
|
|    Written By: James C. Tilton, NASA's GSFC, Mail Code 606.3, Greenbelt, MD 20771
|        E-Mail: James.C.Tilton@nasa.gov
|
|       Written: January 6, 2010
| Modifications: March 24, 2010 - Added class accuracy and average class accuracy calculation
|                June 22, 2010 - Added calculation of Kappa Statistic
|                August 17, 2010: Converted into a general utility program.
|
------------------------------------------------------------*/

#include "contable.h"
#include "params/params.h"
#include <iostream>

extern CommonTilton::Params params;
extern CommonTilton::Image testImage;
extern CommonTilton::Image classifiedImage;

namespace CommonTilton
{
// Compute contingency table
  bool contable()
  {
    int col, row, ncols, nrows;
    int test_value, classified_value, max_value;
    int test_index, classified_index, index;
    unsigned int *contable, *total_for_class, *total_for_test, nb_test_classes = 0;
    float *class_accuracy;

    max_value = (int) testImage.getMaximum(0);
    if (max_value < (int) classifiedImage.getMaximum(0))
      max_value = (int) classifiedImage.getMaximum(0);
    max_value++;

    contable = new unsigned int [max_value*max_value];
    total_for_class = new unsigned int [max_value];
    total_for_test = new unsigned int [max_value];
    class_accuracy = new float [max_value];
    for (classified_index = 0; classified_index < max_value; classified_index++)
    {
      total_for_class[classified_index] = 0;
      total_for_test[classified_index] = 0;
      for (test_index = 0; test_index < max_value; test_index++)
      {
        index = test_index + max_value*classified_index;
        contable[index] = 0;
      }
    }

    ncols = classifiedImage.get_ncols();
    nrows = classifiedImage.get_nrows();
    if ((ncols != testImage.get_ncols()) || (nrows != testImage.get_nrows()))
    {
      cout << "ERROR: Image size mismatch between test image and classified image" << endl;
      return false;
    }
    for (row = 0; row < nrows; row++)
      for (col = 0; col < ncols; col++)
      {
        test_value = (int) testImage.get_data(col,row,0);
        classified_value = (int) classifiedImage.get_data(col,row,0);
        if (test_value > 0)
        {
          total_for_test[test_value] += 1;
          if (classified_value > 0)
          {
            total_for_class[classified_value] += 1;
            index = test_value + max_value*classified_value;
            contable[index] += 1;
          }
        }
      }

    cout << endl << "Contingency Table between Test Set and Classified Result:" << endl << endl;
    cout << "           ";
    for (test_index = 1; test_index < max_value; test_index++)
      if (total_for_test[test_index] > 0)
      {
        nb_test_classes++;
        cout << "   Class " << test_index;
      }
    cout << "   Total" << endl;

    unsigned int total_entries = 0;
    unsigned int matching_entries = 0;
    for (classified_index = 1; classified_index < max_value; classified_index++)
      if (total_for_class[classified_index] > 0)
      {
        cout << "Class " << classified_index << ": ";
        for (test_index = 1; test_index < max_value; test_index++)
          if (total_for_test[test_index] > 0)
          {
            index = test_index + max_value*classified_index;
            cout.width(10);
            cout << contable[index];
            total_entries += contable[index];
            if (test_index == classified_index)
              matching_entries += contable[index];
          }
        cout.width(10);
        cout << total_for_class[classified_index];
        cout << endl;
    }
    cout << "Total  : ";
    for (test_index = 1; test_index < max_value; test_index++)
      if (total_for_test[test_index] > 0)
      {
        cout.width(10);
        cout << total_for_test[test_index];
      }
    cout.width(10);
    cout << total_entries << endl;
    float overall_accuracy = 100*(((float) matching_entries)/((float) total_entries));
    cout << endl << "Percent Overall Accuracy: " << overall_accuracy << endl;

    cout << endl << "Class average accuracies: " << endl;
    float average_accuracy = 0.0;
    cout << "         ";
    for (test_index = 1; test_index < max_value; test_index++)
      if (total_for_test[test_index] > 0)
      {
        index = test_index + max_value*test_index;
        class_accuracy[test_index] = 100*(((float) contable[index])/((float) total_for_test[test_index]));
        cout.width(10);
        cout << class_accuracy[test_index];
        average_accuracy += class_accuracy[test_index];
      }
    cout << endl << endl << "Percent Class Average Accuracy: " << average_accuracy/((float) nb_test_classes) << endl;

    double observed_sum = 0.0;
    double expected_sum = 0.0;
    double kappa;
    for (test_index = 1; test_index < max_value; test_index++)
      if (total_for_test[test_index] > 0)
      {
        index = test_index + max_value*test_index;
        observed_sum += contable[index];
        expected_sum += ((double) total_for_test[test_index])*((double) total_for_class[test_index]);
      }
    kappa = 100*((total_entries*observed_sum) - expected_sum)/((((double) total_entries)*((double) total_entries)) - expected_sum);
    cout << endl << "Kappa Statistic: " << kappa << endl;

    return true;
  }
} // namespace CommonTilton
