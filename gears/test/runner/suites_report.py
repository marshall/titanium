import os
from Cheetah.Template import Template

class SuitesReport:
  """ Create test suites report.
  
  Current implementation writes output in JUnitReport format for CI server
  interpret the data.
  """
  def __init__(self, template_file_name):
    self.template_file_name = template_file_name

  
  def writeReport(self, data, output):
    """ Outputs test result data in test report friendly format.

    The template currently structured to match JUnitReport DTD.

    Args:
      data: structured data with examples in SuiteReportTest
      output: output stream that results will be written
    """
    
    transformed_data = self.groupResultsBySuites(data)
    template = Template(file=self.__template_location(), 
                        searchList={'data': transformed_data})
    output.write(str(template))


  def groupResultsBySuites(self, original_results):
    """ Groups generated test results by suites.

    Sample output can be seen at SuitesReportTest.SAMPLE_OUTPUT.
    """
    transformed_results = {}
    for browser_name in original_results.keys():
      if self.__isTimedoutTest(original_results, browser_name):
        transformed_results[browser_name] = original_results[browser_name]
        continue
      else:
        transformed_results[browser_name] = {}
        self.__copyEnvironmentData(original_results, 
                                   transformed_results, browser_name)
        self.__appendSuiteInfo(transformed_results[browser_name], 
                               original_results[browser_name]["results"])
      
    return transformed_results
  
  def __isTimedoutTest(self, test_data, browser_name):
    return test_data[browser_name] == "TIMED-OUT" 


  def __copyEnvironmentData(self, test_data, result, browser_name):
    for key in ["browser_info", "gears_info", "url"]:
      result[browser_name][key] = test_data[browser_name][key]

    
  def __template_location(self):
      return os.path.join(os.path.dirname(__file__), self.template_file_name)
    
        
  def __appendSuiteInfo(self, store, test_data):
    store["suites"] = {}
    for file_result in test_data:
      suite_name = file_result["suitename"]
      if (suite_name not in store["suites"]):
        store["suites"][suite_name] = {"file_results": [], "elapsed": 0}
      
      # Elapsed time for a suite is the sum of the elapsed times of the files
      store["suites"][suite_name]["elapsed"] += float(file_result["elapsed"])

      store["suites"][suite_name]["file_results"].append( \
          {"filename" : file_result["filename"],
           "results" : file_result["results"]})

    # Leave value as string for consistency
    store["suites"][suite_name]["elapsed"] = \
        str(store["suites"][suite_name]["elapsed"])
