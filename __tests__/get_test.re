[%%raw {|require('isomorphic-fetch')|}];
open Jest;

Nock.disableNetConnect();

describe("get", () => {
  open Expect;

  let scope = Nock.scope("http://example.com");

  afterAll(() => Nock.restore(scope));

  testAsync("get", (done_) => {
    let _: Nock.scope = scope |> Nock.get("/") |> Nock.reply(200, { "foo": 42 });

    Resync.(Refetch.(
      get("http://example.com")
      |> Future.whenResolved((_) => done_(expect(Nock.isDone(scope)) |> toBe(true)))
    ));
  })
});