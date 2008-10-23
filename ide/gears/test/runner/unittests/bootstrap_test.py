import os
import shutil
import StringIO
import runner
import unittest
import pmock
from runner import Bootstrap

class BootstrapTest(unittest.TestCase):
  
  def setUp(self):
    self.__create_output_dir
    self.gears_binaries = "testdata/windows/installers"
    self.testrunner_mock = pmock.Mock()
    self.suites_report_mock = pmock.Mock()
    self.installers = [pmock.Mock(), pmock.Mock()]
    self.bootstrap = Bootstrap(self.gears_binaries, self.installers, 
                               self.testrunner_mock, self.suites_report_mock)


  def tearDown(self):
    self.testrunner_mock.verify()
    self.__delete_output_dir()
    self.suites_report_mock.verify()
    for installer in self.installers:
      installer.verify()

    
  def testStartTesting(self):
    self.testrunner_mock.expects(pmock.once()).runTests()
    self.bootstrap.startTesting()
  
  
  def testReportResultsWritesToFile(self):
    test_result_data = {'browser1': 'TIMED-OUT'}
    self.testrunner_mock.expects(pmock.once()).runTests() \
      .will(pmock.return_value(test_result_data))
    output_stream = StringIO.StringIO()
    self.suites_report_mock.expects(pmock.once()).method("writeReport")
    self.bootstrap.startTesting()
    self.bootstrap.writeResultsToFile()


  def testInstall(self):
    for installer in self.installers:
      installer.expects(pmock.once()).install()
    self.bootstrap.install()


  def testClean(self):
    self.__create_output_dir()
    self.assert_(os.path.exists('output'))
    self.bootstrap.clean()
    self.assert_(not os.path.exists('output'))


  def testCleanDoesNotFailIfOutputDirMissing(self):
    self.__delete_output_dir()
    self.assert_(not os.path.exists('output'))
    self.bootstrap.clean()


  def __create_output_dir(self):
    self.__delete_output_dir()
    os.mkdir('output')
    
    
  def __delete_output_dir(self):   
    if (os.path.exists('output')):
      shutil.rmtree('output')      

    
if __name__ == "__main__":
  unittest.main()
