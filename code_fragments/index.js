//function to resize images on firebase storage
exports.resizeFile = functions.storage.object().onFinalize((object) => {
    const filePath = object.name;
    const fileName = path.basename(filePath);
    const contentType = object.contentType;
    if (!contentType.startsWith('image/')) { 
    //Ends the resizeFile call once firestore detects a metadata change
        return null;
    }      
    const metadata = {
        //set contentType to default
        contentType: contentType,
    };
    //acts as a temp bucket in our google cloud for deletion and download
    const currBucket = gcs.bucket("gs://sya-app.appspot.com")
    const thumbFileName = fileName;
    const thumbFilePath = path.join(path.dirname(filePath), thumbFileName);
    const tmpPath = path.join(os.tmpdir(), path.basename(filePath))
    return currBucket.file(filePath).download({
        destination: tmpPath
    }).then(() => {
        return spawn('convert', [tmpPath,'-resize', '600x600', tmpPath]);
    }).then(() => {
        return currBucket.file(filePath).delete({destination: tmpPath})
         .then(() => {
            return currBucket.upload(tmpPath, {
                destination: thumbFilePath,
                //updates the metadata as a resized jpeg
                contentType: 'jpeg/' 
              })
        })
    });
}); 

/*
This function refunds an artist based on a submission that has not been replied to for atleast 48 hours
We utilize cronjobs to be able to call this function every 48 hours automatically
Also note that this particular code segment left out other functions that will be triggered
in response to a "refund." (Email confirmations to both parties, changes in other forms, etc)
*/

exports.daily_job = functions.https.onRequest((req, res) => { 
    const key = req.query.key;
    
    //Exit if the keys don't match.
    if (!secureCompare(key, functions.config().cron.key)) {
      console.log('The key provided in the request does not match the key set in the environment. Check that', key,
          'matches the cron.key attribute in `firebase env:get`');
      res.status(403).send('Security key does not match. Make sure your "key" URL query parameter matches the ' +
          'cron.key environment variable.');
      return null;
    }

    //Complex query to find particular forms that need to be refunded within subcollections
    let db = admin.firestore()
    const promise = db.collection('review_requests').where('replied', '==', false).where('refunded', '==', 0).get()
    const p2 = promise.then(function(querySnapshot){
        querySnapshot.forEach(function(doc) {
            console.log("Might need refund")
            console.log("submitted on: " + doc.data().submitted_on)
            const now = Date.now()
            //In order to look at a particular instance of 48 hours 
            //The calculation is based off of ms => 1000 (1s) => 60 * 1000 (1m) => 60 * 60 * 1000 (1hr) => 48 * 60 * 60 * 1000 (48 hrs)
            const compareHours = now - (48 * 60 * 60 * 1000)
            const refundedDocId = doc.id
            //compare if the (time now - 48 hours) > (time submitted) ==> if this is true then that means it's been more than 48 hours and no reply
            if(compareHours > doc.data().submitted_on) {
                //Once inside this statement, the document needs a refund
                const artistRefundId = doc.data().art.artist_id
                //used as another promise to set a refund based off of their current # of credits
                var refundRef = db.collection('users').doc(artistRefundId).get()
                const toBeRefunded = refundRef.then(function(doc) {
                    const creditTobeRefunded = doc.data().credits + 1
                    console.log("credit to refund: " + creditTobeRefunded)
                    return db.collection('users').doc(artistRefundId).update({
                        credits: creditTobeRefunded,
                    }).then(() => {
                        //afterwards we have to flag this submission as already refunded to prevent being refunded again
                        return db.collection('review_requests').doc(refundedDocId).update({
                            refunded: 1,
                        })
                    })
                })
                toBeRefunded.catch(error => {
                //error handling for http, we must send a packet back once we know there's an error
                    console.log('error handling ArtistRefundId: ', error)
                    res.send('error with artistRefundId');
                })
            }//loops back to the next document
                })
                //after the loop ends, if everything is successful - this should be sent to the receiving end
                res.send('Finally Refunded all accounts!');
    })
    p2.catch(error => {
        //if we catch any errors pertaining to the documents being the source of the problem
        console.log('Error getting documents: ', error)
        res.send('error');
    })
});
