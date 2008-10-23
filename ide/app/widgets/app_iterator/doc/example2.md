Title: Nested iterators, and formatting dates in an iterator

<style>
<!--
.patron {
  margin: 5px 5px 5px 15px;
  background-color: #eee;
}
.book {
  margin-left: 2px 2px 2px 8px;
}
-->
</style>

This is an example of

++example
Library Loans:
<app:iterator property="loans" on="l:library.loans.transformed.response then execute">
  <html:div class="patron">Patron: #{lastName}, #{firstName}</html:div>
  Books:
  <app:iterator items="#{books}">
    <html:div class="book">
      #{title}, by #{author} (due #{dueDateFormatted})
      <!-- in the 2.1.3 version, we'll get templatable expressions, like: #{Jel.Date.format(dueDate, 'Y/m/d')} -->
    </html:div>
  </app:iterator>
</app:iterator>

<app:script>
/* A listener, that transforms the message payload
 * into a format that should be displayed. This will be obsoleted by the 2.1.3 SDK.
 */
$MQL('l:library.loans.response', function(msgname, data) {
  var loans = data.loans;
  for(var i = 0; i &lt; loans.length; i++) {
    var books = loans[i].books;
    for(var j = 0; j &lt; books.length; j++) {
      books[j].dueDateFormatted = Jel.Date.format(books[j].dueDate, 'Y/m/d');
    }
  }
  $MQ('l:library.loans.transformed.response', data);
});

$MQ('l:library.loans.response', {
  loans: [
    {
      lastName: 'Georges',
      firstName: 'Gabrielle',
      books: [
        {title: 'Krik? Krak!', author: 'Danticat, Edwidge', dueDate: new Date(2008, 6, 11)},
        {title: 'Breath, Eyes, Memory', author: 'Danticat, Edwidge', dueDate: new Date(2008, 6, 11)}
      ]
    },
    {
      lastName: 'Luffel',
      firstName: 'Mark',
      books: [
        {title: 'The Uses of Literature', author: 'Calvino, Italo', dueDate: new Date(2008, 6, 11)},
        {title: 'My Name is Red', author: 'Pamuk, Orhan', dueDate: new Date(2008, 5, 29)}
      ]
    }
  ]
});
</app:script>
--example
