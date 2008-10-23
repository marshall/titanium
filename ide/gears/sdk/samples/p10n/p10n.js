// Copyright 2007, Google Inc.
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//  3. Neither the name of Google Inc. nor the names of its contributors may be
//     used to endorse or promote products derived from this software without
//     specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
// EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

var db, slideNumber, numSlides, resourceStore, visibleSlideCount;

initDatabase();

// Execute a query returning the single result value.
function selectField(db, query, args) {
  var rs = db.execute(query, args);
  var result;
  try {
    result = rs.field(0);
  } finally {
    rs.close();
  }
  return result;
}

/**
 * For each row in the result set f will be called with the following
 * parameters: row (map where the column names are the key) and rowIndex.
 *
 * @param {Object} db The database object
 * @param {String} sql The SQL statement to execute
 * @param {Array} args query params
 * @param {Function} f Function to call for each row
 */
function forEachRow(db, sql, args, f) {
  var rs = db.execute(sql, args);
  try {
    var rowIndex = 0;
    var cols = rs.fieldCount();
    var colNames = [];
    for (var i = 0; i < cols; i++) {
      colNames.push(rs.fieldName(i));
    }

    var rowMap;
    while (rs.isValidRow()) {
      rowMap = {};
      for (var i = 0; i < cols; i++) {
        rowMap[colNames[i]] = rs.field(i);
      }
      f.call(null, rowMap, rowIndex);
      rs.next();
      rowIndex++;
    }
  } finally {
    rs.close();
  }
}

function stripHtml(s) {
  return s.replace(/<[^>]+>/g, '')
          .replace(/&[^;\s]+;?/g, '');
}

// Reload the slides database from database.txt, calling cb() when
// completed.
function databaseLoad(cb) {
  $.getJSON('database.txt', function(json){
    db.execute('BEGIN').close();
    try {
      db.execute('DROP TABLE IF EXISTS Points').close();
      db.execute('DROP TABLE IF EXISTS Slides').close();
      db.execute('DROP TABLE IF EXISTS Search').close();
      db.execute('DROP TABLE IF EXISTS Master').close();
      db.execute('DROP TABLE IF EXISTS Meta').close();

      db.execute('CREATE TABLE Meta (' +
                 '  visibleSlideCount NUMBER' +
                 ')').close();
      db.execute('CREATE TABLE Master (' +
                 '  id INTEGER PRIMARY KEY,' +
                 '  fk_slideid INTEGER NOT NULL' +
                 ')').close();
      db.execute('CREATE TABLE Slides (' +
                 '  id INTEGER PRIMARY KEY,' +
                 '  title TEXT' +
                 ')').close();
      db.execute('CREATE TABLE Points (' +
                 '  fk_slideid INTEGER NOT NULL,' +
                 '  id INTEGER NOT NULL,' +
                 '  content TEXT,' +
                 '  PRIMARY KEY (fk_slideid, id)' +
                 ')').close();
      db.execute('CREATE VIRTUAL TABLE Search USING FTS2 ' +
                 '(content)').close();

      db.execute('INSERT INTO Meta ' +
                 '(visibleSlideCount)' +
                 'VALUES (?)',
                 [json.visibleSlideCount || 0]).close();

      for (var i = 0; i < json.slides.length; i++) {
        var slide = json.slides[i];
        db.execute('INSERT INTO Slides (title, id) VALUES (?, ?)',
                   [slide.title, i]).close();

        db.execute('INSERT INTO Master ' +
                   'VALUES (null, LAST_INSERT_ROWID())').close();

        // add slide title as point -1
        db.execute('INSERT INTO Points ' +
                   '(fk_slideid, id, content) VALUES (?, -1, ?)',
                   [i, slide.title]).close();
        db.execute('INSERT INTO Search (rowid, content) VALUES ' +
                   '(LAST_INSERT_ROWID(), ?)',
                   [stripHtml(slide.title)]).close();

        for (var j = 0; j < slide.points.length; j++) {
          db.execute('INSERT INTO Points ' +
                     '(fk_slideid, id, content) VALUES (?, ?, ?)',
                     [i, j, slide.points[j]]).close();

          db.execute('INSERT INTO Search (rowid, content) VALUES ' +
                     '(LAST_INSERT_ROWID(), ?)',
                     [stripHtml(slide.points[j])]).close();
        }
      }
      db.execute('COMMIT').close();
    } catch (e) {
      db.execute('ROLLBACK').close();
      alert(e.message);
      return;
    }
    if (cb) cb();
  });
}

// Figure out which slide to show based on the querystring.
function render() {
  slideNumber = parseInt(location.search.substring(1));
  if (isNaN(slideNumber)) {
    slideNumber = -1;
    // reload the database on every home display.
    // databaseLoad is async, so we pass renderEnd as the callback.
    databaseLoad(renderEnd);
  } else {
    renderEnd();
  }
}

function renderEnd() {
  numSlides = selectField(
      db,
      'SELECT COUNT(*) ' +
      'FROM Master JOIN Slides ON Master.fk_slideid=Slides.id'
      );

  visibleSlideCount = selectField(
      db,
      'SELECT visibleSlideCount FROM Meta'
      );

  if (visibleSlideCount <= 0) {
    visibleSlideCount = numSlides;
  }

  if (-1 == slideNumber) {
    homeDisplay();
  } else if (slideNumber == numSlides) {
    lastDisplay();
  } else {
    slideDisplay(slideNumber);
  }

  $('#num_slides').html(visibleSlideCount);

  footerDisplay();

  if (resourceStore && resourceStore.currentVersion) {
    displayStatus('Captured', 'green');
  }
}

function displayStatus(message, color) {
  $('#status').html('<span class=' + color + '>' + message + '</span>');
}

function homeDisplay() {
  $('.place#body').html($('#home'));
}

function lastDisplay() {
  $('.place#body').html($('#last'));
}

function footerDisplay() {
  // populate the footer
  $('#slide_number').html(String(slideNumber + 1));
  // +1 because of "last" slide which isn't in db.
  $('#num_slides').html(String(visibleSlideCount + 1));
}

// Display the ii'th slide in the Master ordering of the stack.
function slideDisplay(ii) {

  if (ii < 0 || ii >= numSlides) {
    // We tried to show a page that is out of bounds... go to the main page.
    slideNumber = -1;
    homeDisplay();
    return;
  }

  var id = selectField(
      db,
      'SELECT Slides.id ' +
      'FROM Master JOIN Slides ON Master.fk_slideid=Slides.id ' +
      'ORDER BY Master.id LIMIT ?, 1', [ii]);

  var slideTemplate = $('#slide-template');
  var clone = slideTemplate.clone(true).removeClass('template');
  clone.find('h1').html(
      selectField(db, 'SELECT title FROM Slides WHERE id=?', [id]));

  var li = clone.find('li:first');
  forEachRow(db, 'SELECT content FROM Points ' +
                 'WHERE fk_slideid=? AND id>=0 ' +
                 'ORDER BY id', [id], function(map, index) {

    var liClone = li.clone(true).html(map['content']);
    li.before(liClone);
  });
  li.remove();

  // Insert the slide
  $('.place#body').html(clone);

  // Syntax highlight pre elements
  var pres = $('.place#body pre');
  for (var i = 0, pre; pre = pres[i]; i++) {
    pre.innerHTML =
        '<span class=hoverable>' +
        pre.innerHTML.replace(/('[^']+')/g, '<span class=string>$1</span>')
                     .replace(/((var)|(while)|(function))/g,
                              '<span class=keyword>$1</span>')
                     .replace(/<br>/ig, '</span><br><span class=hoverable>') +
        '</span>';
  }
}

// Quote special characters for dump.
function jsquote(s) {
  s = s.replace(/\"/g, '\\"');
  //s = s.replace(/\&/g, '\&amp;');
  s = s.replace(/\n/g, '\\n');
  return s;
}

// Emit a json dump of the database into .place#dump.
function databaseDump() {
  var rs;
  var slides = [];

  var out = '{\n';

  rs = db.execute(
      'SELECT Slides.id, title ' +
      'FROM Master JOIN Slides ON Master.fk_slideid=Slides.id' +
      'ORDER BY Master.id');
  while (rs.isValidRow()) {
    var id = rs.field(0);
    var title = rs.field(1);

    var slide ='{ "title": "' + jsquote(title) + '",\n';

    var points = [];

    var prs = db.execute('SELECT content FROM Points ' +
                         'WHERE fk_slideid=? ORDER BY id',
                         [id]);
    while (prs.isValidRow()) {
      points[points.length] = jsquote(prs.field(0));
      prs.next();
    }
    prs.close();

    if (points.length) {
      slide += '      "points": [\n' +
               '        "' +
               points.join('",\n        "') +
               '"\n' +
               '      ],\n';
    } else {
      slide += '      "points": [],\n';
    }

    slide += '    }';

    slides.push(slide);

    rs.next();
  }
  rs.close();

  if (slides.length) {
    out += '  "slides": [\n' +
           '    ' +
           slides.join(',\n    ') +
           '\n  ]';
  } else {
    out += '  "slides": []';
  }
  out += '\n' +
         '}\n';
  $('.place#dump').text(out);
}

// Capture the presentation.
function capture() {
  var localServer = google.gears.factory.create('beta.localserver');
  localServer.removeManagedStore('slides');
  resourceStore = localServer.createManagedStore('slides');
  resourceStore.manifestUrl = 'p10n_manifest.txt';
  displayStatus('Capturing. Please wait...', 'orange');
  resourceStore.checkForUpdate();

  var timerId = window.setInterval(function() {
    if (resourceStore.updateStatus == 3) {
      window.clearInterval(timerId);
      // If this happens during my presentation, I die.
      displayStatus('Failed to capture: ' + resourceStore.lastErrorMessage,
                    'red');
      return;
    }

    if (resourceStore.currentVersion) {
      window.clearInterval(timerId);

      // put a little extra delay in so it's easier to see.
      window.setTimeout(function() {
        displayStatus('Captured', 'green');
      }, 1000);
    }
  }, 500);
}

$('html').bind('keydown', function(e) {

  // If the user is typing in a text field we don't want to handle keyboard
  // shortcuts.
  if (/textarea|input/i.test(e.target.tagName) ||
      e.altKey || e.ctrlKey || e.metaKey) {
    return true;
  }

  switch (e.keyCode) {
    // enter, space, right arrow, down arrow
    case 13:
    case 32:
    case 39:
    case 40:
      next();
      return false;
    // backspace, left arrow, up arrow
    case 8:
    case 37:
    case 38:
      prev();
      return false;
  }

  return true;
});

function next() {
  if (slideNumber < numSlides) {
    location.href = location.pathname + '?' + (slideNumber + 1);
  }
}

function prev() {
  if (slideNumber == 0) {
    location.href = location.pathname;
  } else if (slideNumber > 0) {
    location.href = location.pathname + '?' + (slideNumber - 1);
  }
}

function initDatabase() {
  try {
    db = google.gears.factory.create('beta.database');
    db.open('slides');
  } catch(e) {
    $('.place#body').text('Cannot create database');
    alert('Sorry, cannot create database:\n\n' + e.message);
    return;
  }

  try {
    var server = google.gears.factory.create('beta.localserver');
    resourceStore = server.openManagedStore('slides');
  } catch (e) {
    alert('Could not create localserver');
  }
}

function search() {
  var q = $('#query').val();
  displaySearchResult(q);
  return false;
}

function displaySearchResult(q) {
  var base = window.location.href.split('?')[0] + '?';
  var displayUrlBase = base.replace(/^http(?:s)?:\/\//i, '');

  var searchTemplate = $('#search-template');
  var searchResult = $('#search-result').empty();

  forEachRow(db, 'SELECT title, snippet(Search) as content, ' +
                 'p.fk_slideid as slideid ' +
                 'FROM Search s, Points p, Slides ' +
                 'WHERE p.rowid=s.rowid AND Search MATCH ? AND ' +
                 'p.fk_slideid=Slides.id ' +
                 'LIMIT 5',
                 [q], function(map, index) {

    var clone = searchTemplate.clone(true).removeClass('template');

    clone.find('h2>a').attr('href', base + (map['slideid']))
        .html(map['title']);
    clone.find('p:first').html(map['content']);
    clone.find('p.url').text(displayUrlBase + (map['slideid']));
    searchResult.append(clone);
  });
}
